#pragma once
#include <cstdint>
#include <span>
namespace aob {
constexpr uint32_t kMagic=0x414F4250u; constexpr uint8_t kVersionMajor=1,kVersionMinor=0; constexpr size_t kMaxPayload=4096;
enum class PacketType:uint8_t{Audio=1,Hello=2,Control=3,Telemetry=4,Ack=5};
struct AudioPacket{uint32_t stream_id{};uint64_t sequence{};uint64_t timestamp_frames{};uint32_t sample_rate{48000};uint16_t channels{2};uint16_t frame_ms{20};std::span<const uint8_t> payload{};};
size_t serialize_audio(const AudioPacket&,std::span<uint8_t>);
bool deserialize_audio(std::span<const uint8_t>,AudioPacket&,std::span<const uint8_t>&);
}