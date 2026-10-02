#include "aob/packet.hpp"
#include <array>
#include <cassert>
int main(){std::array<uint8_t,256>b{};std::array<uint8_t,5>x{1,2,3,4,5};aob::AudioPacket p{7,99,480,48000,2,20,x};auto n=aob::serialize_audio(p,b);assert(n);aob::AudioPacket q{};std::span<const uint8_t>pl;assert(aob::deserialize_audio({b.data(),n},q,pl));assert(q.sequence==99&&pl[2]==3);b[50]^=1;assert(!aob::deserialize_audio({b.data(),n},q,pl));}