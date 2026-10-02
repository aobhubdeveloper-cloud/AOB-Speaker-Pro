#include <jni.h>
#include <atomic>
#include <array>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <opus/opus.h>
#include <oboe/Oboe.h>
#include "aob/packet.hpp"
static std::atomic<bool>running{false};static int sockfd=-1;static std::thread rx;
struct Frame{std::array<float,1920>pcm{};};static std::mutex qmx;static std::condition_variable qcv;static std::queue<Frame>q;
class Callback:public oboe::AudioStreamDataCallback{public:oboe::DataCallbackResult onAudioReady(oboe::AudioStream*,void*d,int32_t n)override{float*out=(float*)d;std::fill(out,out+n*2,0.f);std::unique_lock<std::mutex>l(qmx);if(!q.empty()){auto f=std::move(q.front());q.pop();l.unlock();int c=std::min(n*2,1920);std::copy(f.pcm.begin(),f.pcm.begin()+c,out);}return oboe::DataCallbackResult::Continue;}};
static oboe::ManagedStream stream;
static void receiver(int port){sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=INADDR_ANY;a.sin_port=htons(port);sockfd=socket(AF_INET,SOCK_DGRAM,0);if(sockfd<0)return;bind(sockfd,(sockaddr*)&a,sizeof(a));int e=0;OpusDecoder*d=opus_decoder_create(48000,2,&e);std::array<uint8_t,65536>buf{};std::array<float,1920>pcm{};
while(running){sockaddr_in from{};socklen_t fl=sizeof(from);auto n=recvfrom(sockfd,(char*)buf.data(),buf.size(),0,(sockaddr*)&from,&fl);if(n<=0)continue;aob::AudioPacket p{};std::span<const uint8_t>pl;if(!aob::deserialize_audio({buf.data(),size_t(n)},p,pl)||p.sample_rate!=48000||p.channels!=2)continue;int samples=opus_decode_float(d,pl.data(),int(pl.size()),pcm.data(),960,0);if(samples==960){Frame f;std::copy(pcm.begin(),pcm.end(),f.pcm.begin());std::lock_guard<std::mutex>l(qmx);if(q.size()<8)q.push(std::move(f));qcv.notify_one();}}
opus_decoder_destroy(d);close(sockfd);sockfd=-1;}
extern "C" JNIEXPORT jboolean JNICALL Java_com_aobhub_speakerpro_AudioReceiverService_nativeStart(JNIEnv*,jobject,jint port){if(running)return JNI_TRUE;oboe::AudioStreamBuilder b;b.setDirection(oboe::Direction::Output).setPerformanceMode(oboe::PerformanceMode::LowLatency).setSharingMode(oboe::SharingMode::Exclusive).setFormat(oboe::AudioFormat::Float).setChannelCount(2);static Callback cb;b.setDataCallback(&cb);if(b.openManagedStream(stream)!=oboe::Result::OK)return JNI_FALSE;if(stream->requestStart()!=oboe::Result::OK)return JNI_FALSE;running=true;rx=std::thread(receiver,port);return JNI_TRUE;}
extern "C" JNIEXPORT void JNICALL Java_com_aobhub_speakerpro_AudioReceiverService_nativeStop(JNIEnv*,jobject){running=false;if(sockfd>=0)shutdown(sockfd,SHUT_RDWR);if(rx.joinable())rx.join();if(stream)stream->requestStop();stream.reset();std::lock_guard<std::mutex>l(qmx);std::queue<Frame>empty;std::swap(q,empty);}