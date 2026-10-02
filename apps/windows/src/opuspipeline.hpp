#pragma once
#include <cstddef>
#include <span>
namespace aob{class OpusPipeline{void*e_=nullptr;int ch_=2;public:bool open(int sr=48000,int ch=2,int bitrate=128000);size_t encode(std::span<const float>,std::span<unsigned char>);void close();};}