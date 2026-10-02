#include <jni.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <opus/opus.h>
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
static std::atomic<uint32_t> head{0}, tail{0};

class Callback : public oboe::AudioStreamDataCallback {
public:
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* stream, void* data, int32_t numFrames) override {
        float* out = static_cast<float*>(data);
        std::fill(out, out + numFrames * 2, 0.f);

        auto t = tail.load(std::memory_order_relaxed);
        if (t != head.load(std::memory_order_acquire)) {
            const auto& frame = ring[t % RING];
            std::copy(frame.pcm.begin(), frame.pcm.end(), out);
            tail.store((t + 1) % RING, std::memory_order_release);
        }

        return oboe::DataCallbackResult::Continue;
    }
};

static oboe::ManagedStream stream;

static void receiver(int port) {
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_port = htons(port);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return;

    if (bind(sockfd, reinterpret_cast<sockaddr*>(&a), sizeof(a)) < 0) {
        close(sockfd);
        sockfd = -1;
        return;
    }

    OpusDecoder* decoder = opus_decoder_create(48000, 2, nullptr);
    if (!decoder) {
        close(sockfd);
        sockfd = -1;
        return;
    }

    std::vector<uint8_t> buffer(8192);
    while (running) {
        sockaddr_in from{};
        socklen_t fromLen = sizeof(from);
        const auto n = recvfrom(sockfd,
                                reinterpret_cast<char*>(buffer.data()),
                                static_cast<int>(buffer.size()),
                                0,
                                reinterpret_cast<sockaddr*>(&from),
                                &fromLen);

        if (n <= 0) continue;

        aob::AudioPacket packet{};
        std::span<const uint8_t> bytes(buffer.data(), static_cast<size_t>(n));
        if (packet.decode(bytes)) {
            const uint32_t idx = head.load(std::memory_order_relaxed);
            auto& frame = ring[idx % RING];
            std::fill(frame.pcm.begin(), frame.pcm.end(), 0.f);
            head.store((idx + 1) % RING, std::memory_order_release);
        }
    }

    opus_decoder_destroy(decoder);
    close(sockfd);
    sockfd = -1;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_aobhub_speakerpro_AudioReceiverService_nativeStart(JNIEnv*, jobject, jint port) {
    if (running) return JNI_TRUE;

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Output);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setChannelCount(2);
    builder.setSampleRate(48000);
    builder.setFormat(oboe::AudioFormat::Float);

    const oboe::Result result = builder.openStream(&stream);
    if (result != oboe::Result::OK) {
        return JNI_FALSE;
    }

    running = true;
    rx = std::thread(receiver, port);
    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL Java_com_aobhub_speakerpro_AudioReceiverService_nativeStop(JNIEnv*, jobject) {
    running = false;
    if (sockfd >= 0) shutdown(sockfd, SHUT_RDWR);
    if (rx.joinable()) rx.join();
    if (stream) {
        stream->close();
    }
}
