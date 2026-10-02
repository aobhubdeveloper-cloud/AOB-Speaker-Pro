#pragma once
#include <cstdint>
#include <string>
#include <span>
class UdpSender{uintptr_t s_=~uintptr_t(0);public:bool open(const std::string&,uint16_t);bool send(std::span<const uint8_t>);void close();};