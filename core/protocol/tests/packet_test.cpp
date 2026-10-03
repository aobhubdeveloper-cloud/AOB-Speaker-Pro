#include "aob/packet.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <span>
#include <vector>

int main() {
    using namespace aob;

    std::array<uint8_t, 512> buffer{};
    const std::array<uint8_t, 5> payload{1, 2, 3, 4, 5};
    AudioPacket input{7, 99, 480, 48000, 2, 20, payload};

    const auto size = serialize_audio(input, buffer);
    assert(size == 44 + payload.size());

    AudioPacket output{};
    std::span<const uint8_t> decoded{};
    assert(deserialize_audio({buffer.data(), size}, output, decoded));
    assert(output.stream_id == 7);
    assert(output.sequence == 99);
    assert(output.timestamp_frames == 480);
    assert(output.sample_rate == 48000);
    assert(output.channels == 2);
    assert(output.frame_ms == 20);
    assert(decoded.size() == payload.size());
    assert(decoded[2] == 3);

    // Truncated packet must be rejected.
    assert(!deserialize_audio({buffer.data(), size - 1}, output, decoded));

    // Payload corruption must be detected by the checksum.
    auto corrupted = buffer;
    corrupted[44 + 2] ^= 0x01;
    assert(!deserialize_audio({corrupted.data(), size}, output, decoded));

    // Unsupported protocol major version must be rejected.
    auto bad_version = buffer;
    bad_version[4] = 2;
    assert(!deserialize_audio({bad_version.data(), size}, output, decoded));

    // Unsupported packet version marker must be rejected.
    auto bad_packet_version = buffer;
    bad_packet_version[6] = 2;
    assert(!deserialize_audio({bad_packet_version.data(), size}, output, decoded));

    // Invalid magic must be rejected.
    auto bad_magic = buffer;
    bad_magic[0] ^= 0xFF;
    assert(!deserialize_audio({bad_magic.data(), size}, output, decoded));

    // Declared payload larger than the actual input must be rejected.
    auto bad_length = buffer;
    bad_length[39] = 0xFF;
    assert(!deserialize_audio({bad_length.data(), size}, output, decoded));

    // Serialization must reject payloads above the protocol limit.
    std::vector<uint8_t> oversized(kMaxPayload + 1, 0x55);
    AudioPacket too_large{1, 1, 1, 48000, 2, 20, oversized};
    assert(serialize_audio(too_large, buffer) == 0);

    // Serialization must reject an output buffer that is too small.
    std::array<uint8_t, 48> tiny{};
    assert(serialize_audio(input, tiny) == 0);

    return 0;
}
