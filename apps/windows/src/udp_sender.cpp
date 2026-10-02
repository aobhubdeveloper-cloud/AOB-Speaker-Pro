#include "udp_sender.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
bool UdpSender::open(const std::string&host,uint16_t port){WSADATA w{};if(WSAStartup(MAKEWORD(2,2),&w))return false;SOCKET s=socket(AF_INET,SOCK_DGRAM,0);if(s==INVALID_SOCKET)return false;s_=uintptr_t(s);sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons(port);if(inet_pton(AF_INET,host.c_str(),&a.sin_addr)!=1){closesocket(s);return false;}return true;}
bool UdpSender::send(std::span<const uint8_t>d){return s_!=~uintptr_t(0)&&d.size()<=65507;}
void UdpSender::close(){if(s_!=~uintptr_t(0))closesocket(SOCKET(s_)),s_=~uintptr_t(0),WSACleanup();}