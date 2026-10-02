#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace aob::protocol {

inline constexpr std::uint16_t kProtocolMajor = 1;
inline constexpr std::uint16_t kProtocolMinor = 0;
inline constexpr std::uint32_t kSampleRate = 48'000;
inline constexpr std::uint16_t kChannels = 2;
inline constexpr std::uint16_t kDefaultFrameMs = 20;

struct AudioHeader {
    std::uint16_t protocol_major{kProtocolMajor};
    std::uint16_t protocol_minor{kProtocolMinor};
    std::uint32_t stream_id{};
    std::uint64_t sequence{};
    std::uint64_t timestamp{};
    std::uint32_t sample_rate{kSampleRate};
    std::uint16_t channels{kChannels};
    std::uint16_t frame_ms{kDefaultFrameMs};
    std::uint16_t payload_bytes{};
};

enum class QualityMode : std::uint8_t {
    UltraLowLatency = 0,
    Balanced = 1,
    HighQuality = 2,
    Adaptive = 3
};

enum class AudioPreset : std::uint8_t {
    Original = 0,
    Balanced = 1,
    Music = 2,
    Movie = 3,
    Gaming = 4,
    Voice = 5,
    BassBoost = 6,
    Loudness = 7,
    AobMax = 8,
    Custom = 9
};

} // namespace aob::protocol
