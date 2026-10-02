#include "wasapi_loopback.hpp"
#include "opuspipeline.hpp"
#include "udp_sender.hpp"
#include "aob/packet.hpp"
#include <iostream>
#include <vector>
#include <mutex>
int main(int argc,char**argv){
 std::string host=argc>1?argv[1]:"192.168.1.100"; UdpSender udp; if(!udp.open(host,4677)){std::cerr<<"Unable to open UDP transport\n";return 2;}
 aob::OpusPipeline opus;if(!opus.open())return 3; std::vector<float>fifo;fifo.reserve(960*2*4);uint64_t seq=0,timestamp=0;std::mutex m;
 WasapiLoopback cap;if(!cap.start([&](const float*pcm,uint32_t frames,uint32_t channels){
   if(channels!=2)return;std::lock_guard<std::mutex>lock(m);fifo.insert(fifo.end(),pcm,pcm+size_t(frames)*2);
   while(fifo.size()>=1920){std::array<uint8_t,4096>encoded{};auto n=opus.encode({fifo.data(),1920},encoded);if(n){std::array<uint8_t,4096>packet{};aob::AudioPacket p{1,seq++,timestamp,48000,2,20,{encoded.data(),n}};auto sz=aob::serialize_audio(p,packet);if(sz)udp.send({packet.data(),sz});timestamp+=960;}fifo.erase(fifo.begin(),fifo.begin()+1920);}
 }))return 4;
 std::cout<<"AOB Speaker Pro streaming to "<<host<<":4677. Press Enter to stop.\n";std::cin.get();cap.stop();opus.close();udp.close();return 0;
}