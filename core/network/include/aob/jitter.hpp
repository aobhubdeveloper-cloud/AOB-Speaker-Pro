#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
namespace aob{
template<size_t MaxFrames=32>class JitterBuffer{
 struct Slot{uint64_t seq=0;std::array<float,1920>pcm{};bool ready=false;};
 std::array<Slot,MaxFrames>slots_{};uint64_t expected_=0;
 public:bool push(uint64_t seq,const float*p,size_t n){if(!p||n>1920||seq<expected_)return false;auto&i=slots_[seq%MaxFrames];if(i.ready&&i.seq!=seq)return false;i.seq=seq;std::copy(p,p+n,i.pcm.begin());if(n<1920)std::fill(i.pcm.begin()+n,i.pcm.end(),0);i.ready=true;return true;}
 std::optional<std::array<float,1920>> pop(){auto&i=slots_[expected_%MaxFrames];if(!i.ready||i.seq!=expected_)return std::nullopt;auto out=i.pcm;i.ready=false;++expected_;return out;}
 void reset(uint64_t s){expected_=s;for(auto&i:slots_)i.ready=false;}
};}