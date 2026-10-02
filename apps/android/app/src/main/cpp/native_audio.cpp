#include <jni.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <span>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <opus.h>
#include <oboe/Oboe.h>
#include "aob/packet.hpp"

static std::atomic<bool> running{false};
static int sockfd = -1;
static std::thread rx;

struct Frame {
    std::array<float, 1920> pcm{};
};

static constexpr uint32_t RING = 16;
static std::array<Frame, RING> ring{};
static std::atomic<uint32_t> head{0};
static std::atomic<uint32_t> tail{0};

class Callback final : public oboe::AudioStreamDataCallback {
public:
    oboe::DataCallbackResult onAudioReady(
            oboe::AudioStream*, void* data, int32_t numFrames) override {
        auto* out = static_cast<float*>(data);
        std::fill(out, out + numFrames * 2, 0.0f);

        const uint32_t t = tail.load(std::memory_order_relaxed);
        const uint32_t h = head.load(std::memory_order_acquire);
        if (t != h) {
            const auto& frame = ring[t % RING];
            const int samples = std::min(numFrames * 2, 1920);
            std::copy_n(frame.pcm.begin(), samples, out);
            tail.store(t + 1, std::memory_order_release);
        }

        return oboe::DataCallbackResult::Continue;
    }
};

static oboe::ManagedStream stream;
static Callback callback;

static void receiver(int port) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<uint16_t>(port));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return;

    if (bind(sockfd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        close(sockfd);
        sockfd = -1;
        return;
    }

    int opus_error = OPUS_OK;
    OpusDecoder* decoder = opus_decoder_create(48000, 2, &opus_error);
    if (!decoder || opus_error != OPUS_OK) {
        if (decoder) opus_decoder_destroy(decoder);
        close(sockfd);
        sockfd = -1;
        return;
    }

    std::array<uint8_t, 8192> buffer{};
    std::array<float, 1920> pcm{};

    while (running) {
        sockaddr_in from{};
        socklen_t from_len = sizeof(from);
        const auto received = recvfrom(
            sockfd,
            reinterpret_cast<char*>(buffer.data()),
            static_cast<int>(buffer.size()),
            0,
            reinterpret_cast<sockaddr*>(&from),
            &from_len);

        if (received <= 0) continue;

        aob::AudioPacket packet{};
        std::span<const uint8_t> payload;
        const std::span<const uint8_t> datagram(
            buffer.data(), static_cast<size_t>(received));

        if (!aob::deserialize_audio(datagram, packet, payload)) continue;
        if (packet.sample_rate != 48000 || packet.channels != 2) continue;

        const int decoded_frames = opus_decode_float(
            decoder,
            payload.data(),
            static_cast<opus_int32>(payload.size()),
            pcm.data(),
            960,
            0);

        if (decoded_frames != 960) continue;

        const uint32_t h = head.load(std::memory_order_relaxed);
        const uint32_t t = tail.load(std::memory_order_acquire);

        if (h - t >= RING - 1) continue;

        std::copy(pcm.begin(), pcm.end(), ring[h % RING].pcm.begin());
        head.store(h + 1, std::memory_order_release);
    }

    opus_decoder_destroy(decoder);
    close(sockfd);
    sockfd = -1;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_aobhub_speakerpro_AudioReceiverService_nativeStart(
        JNIEnv*, jobject, jint port) {
    if (running.load(std::memory_order_acquire)) return JNI_TRUE;

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);
    builder.setDataCallback(&callback);

    const oboe::Result result = builder.openStream(&stream);
    if (result != oboe::Result::OK) return JNI_FALSE;

    if (stream->requestStart() != oboe::Result::OK) {
        stream->close();
        return JNI_FALSE;
    }

    head.store(0, std::memory_order_release);
    tail.store(0, std::memory_order_release);
    running.store(true, std::memory_order_release);
    rx = std::thread(receiver, static_cast<int>(port));
    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_aobhub_speakerpro_AudioReceiverService_nativeStop(
        JNIEnv*, jobject) {
    running.store(false, std::memory_order_release);

    if (sockfd >= 0) shutdown(sockfd, SHUT_RDWR);
    if (rx.joinable()) rx.join();

    if (stream) {
        stream->requestStop();
        stream->close();
    }
}
