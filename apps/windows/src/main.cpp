#include "wasapi_loopback.hpp"
#include "opuspipeline.hpp"
#include "udp_sender.hpp"
#include "aob/packet.hpp"
#include <opus/opus.h>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
bool self_test() {
    constexpr int kRate = 48000;
    constexpr int kChannels = 2;
    constexpr int kFrames = 960;

    aob::OpusPipeline encoder;
    if (!encoder.open(kRate, kChannels, 128000)) return false;

    std::array<float, kFrames * kChannels> pcm{};
    for (int i = 0; i < kFrames; ++i) {
        const float v = 0.2f * std::sin(2.0f * 3.14159265358979323846f * 440.0f * i / kRate);
        pcm[2 * i] = v;
        pcm[2 * i + 1] = v;
    }

    std::array<uint8_t, 4096> encoded{};
    const auto encoded_size = encoder.encode(pcm, encoded);
    if (encoded_size == 0) return false;

    std::array<uint8_t, 4096> datagram{};
    const aob::AudioPacket source{
        1, 7, 0, static_cast<uint32_t>(kRate), kChannels, 20,
        {encoded.data(), encoded_size}
    };
    const auto packet_size = aob::serialize_audio(source, datagram);
    if (packet_size == 0) return false;

    aob::AudioPacket parsed{};
    std::span<const uint8_t> payload{};
    if (!aob::deserialize_audio({datagram.data(), packet_size}, parsed, payload)) return false;

    int error = OPUS_OK;
    auto* decoder = opus_decoder_create(kRate, kChannels, &error);
    if (!decoder || error != OPUS_OK) return false;

    std::array<float, kFrames * kChannels> decoded{};
    const int decoded_frames = opus_decode_float(
        decoder, payload.data(), static_cast<opus_int32>(payload.size()),
        decoded.data(), kFrames, 0);
    opus_decoder_destroy(decoder);
    encoder.close();

    if (decoded_frames != kFrames) return false;
    for (float sample : decoded) {
        if (!std::isfinite(sample) || std::abs(sample) > 1.0f) return false;
    }
    return true;
}
}

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--self-test") {
        const bool ok = self_test();
        std::cout << (ok ? "AOB self-test: PASS\n" : "AOB self-test: FAIL\n");
        return ok ? 0 : 10;
    }

    const std::string host = argc > 1 ? argv[1] :
        (std::getenv("AOB_SPEAKER_HOST") ? std::getenv("AOB_SPEAKER_HOST") : "");
    const uint16_t port = argc > 2 ? static_cast<uint16_t>(std::stoi(argv[2])) : 4677;

    if (host.empty()) {
        std::cerr << "Usage: aob-speaker-pro <android-ip> [port]\n"
                  << "       aob-speaker-pro --self-test\n";
        return 2;
    }

    UdpSender udp;
    if (!udp.open(host, port)) {
        std::cerr << "Unable to open UDP transport to " << host << ":" << port << "\n";
        return 2;
    }

    aob::OpusPipeline opus;
    if (!opus.open()) return 3;

    std::vector<float> fifo;
    fifo.reserve(3840);
    uint64_t seq = 0;
    uint64_t timestamp = 0;

    WasapiLoopback cap;
    if (!cap.start([&](const float* pcm, uint32_t frames, uint32_t channels, uint32_t sample_rate) {
        if (channels != 2 || sample_rate != 48000) return;

        fifo.insert(fifo.end(), pcm, pcm + static_cast<size_t>(frames) * 2);
        while (fifo.size() >= 1920) {
            std::array<uint8_t, 4096> encoded{};
            const auto n = opus.encode({fifo.data(), 1920}, encoded);
            if (n) {
                std::array<uint8_t, 4096> packet{};
                const aob::AudioPacket p{
                    1, seq++, timestamp, 48000, 2, 20,
                    {encoded.data(), n}
                };
                const auto size = aob::serialize_audio(p, packet);
                if (size && !udp.send({packet.data(), size}))
                    std::cerr << "UDP send failed\n";
                timestamp += 960;
            }
            fifo.erase(fifo.begin(), fifo.begin() + 1920);
        }
    })) return 4;

    std::cout << "AOB Speaker Pro streaming to " << host << ":" << port
              << ". Press Enter to stop.\n";
    std::cin.get();
    cap.stop();
    opus.close();
    udp.close();
    return 0;
}
