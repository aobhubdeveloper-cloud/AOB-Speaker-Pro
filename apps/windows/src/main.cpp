#include "wasapi_loopback.hpp"
#include "opuspipeline.hpp"
#include "udp_sender.hpp"
#include "aob/packet.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

static constexpr uint16_t kAudioPort = 4677;
static constexpr uint16_t kDiscoveryPort = 4678;
static constexpr char kHello[] = "AOB_ANDROID_HELLO_V1";

static bool discover_android(std::atomic<bool>& running, std::string& host) {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) return false;
    BOOL yes = TRUE;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&yes), sizeof(yes));
    sockaddr_in bindAddr{}; bindAddr.sin_family=AF_INET; bindAddr.sin_addr.s_addr=htonl(INADDR_ANY); bindAddr.sin_port=htons(kDiscoveryPort);
    if (bind(s, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) == SOCKET_ERROR) { closesocket(s); return false; }
    DWORD timeout=500; setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
    char buf[256]{};
    while (running) {
        sockaddr_in from{}; int len=sizeof(from);
        int n=recvfrom(s,buf,sizeof(buf)-1,0,reinterpret_cast<sockaddr*>(&from),&len);
        if(n>0 && std::string(buf,n).rfind(kHello,0)==0) {
            char ip[INET_ADDRSTRLEN]{};
            inet_ntop(AF_INET,&from.sin_addr,ip,sizeof(ip));
            host=ip; closesocket(s); return true;
        }
    }
    closesocket(s); return false;
}

int main(int argc,char**argv){
    std::atomic<bool> running{true};
    WSADATA w{}; if(WSAStartup(MAKEWORD(2,2),&w)) { std::cerr<<"Winsock startup failed\n"; return 2; }

    std::string host = argc>1 ? argv[1] : "";
    std::cout<<"AOB Speaker Pro Windows sender started.\n";
    std::cout<<"Waiting for an Android receiver on the same Wi-Fi...\n";

    while(running) {
        if(host.empty()) {
            if(!discover_android(running,host)) break;
            std::cout<<"Android receiver found at "<<host<<". Starting audio...\n";
        }

        UdpSender udp;
        if(!udp.open(host,kAudioPort)) {
            std::cerr<<"Could not open audio transport; waiting for receiver...\n";
            host.clear(); std::this_thread::sleep_for(std::chrono::seconds(2)); continue;
        }

        aob::OpusPipeline opus;
        if(!opus.open()) { std::cerr<<"Opus initialization failed\n"; udp.close(); break; }

        std::vector<float> fifo; fifo.reserve(3840);
        uint64_t seq=0,timestamp=0; std::mutex m;
        WasapiLoopback cap;
        if(!cap.start([&](const float*pcm,uint32_t frames,uint32_t channels){
            if(channels!=2)return;
            std::lock_guard<std::mutex> lock(m);
            fifo.insert(fifo.end(),pcm,pcm+size_t(frames)*2);
            while(fifo.size()>=1920){
                std::array<uint8_t,4096> encoded{};
                auto n=opus.encode({fifo.data(),1920},encoded);
                if(n){
                    std::array<uint8_t,4096> packet{};
                    aob::AudioPacket p{1,seq++,timestamp,48000,2,20,{encoded.data(),n}};
                    auto sz=aob::serialize_audio(p,packet);
                    if(!sz || !udp.send({packet.data(),sz})) {
                        std::cerr<<"Audio send failed; receiver may have disconnected.\n";
                    }
                    timestamp+=960;
                }
                fifo.erase(fifo.begin(),fifo.begin()+1920);
            }
        })) {
            std::cerr<<"WASAPI loopback could not start. Check the Windows playback device.\n";
            opus.close(); udp.close(); host.clear(); std::this_thread::sleep_for(std::chrono::seconds(2)); continue;
        }

        std::cout<<"Streaming PC audio to Android. Press Enter to stop.\n";
        std::cin.get();
        running=false;
        cap.stop(); opus.close(); udp.close();
    }

    WSACleanup();
    return 0;
}