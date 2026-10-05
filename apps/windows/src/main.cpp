#include "wasapi_loopback.hpp"
#include "opuspipeline.hpp"
#include "udp_sender.hpp"
#include "aob/packet.hpp"
#include <opus/opus.h>
#include <windows.h>
#include <commctrl.h>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <atomic>

namespace {
constexpr int IDC_HOST=1001, IDC_PORT=1002, IDC_START=1003, IDC_STOP=1004, IDC_TEST=1005, IDC_STATUS=1006;
HWND gHost=nullptr, gPort=nullptr, gStatus=nullptr;
std::unique_ptr<UdpSender> gUdp;
std::unique_ptr<aob::OpusPipeline> gOpus;
std::unique_ptr<WasapiLoopback> gCapture;
std::vector<float> gFifo;
std::atomic<uint64_t> gSeq{0}, gTimestamp{0};

bool self_test() {
    constexpr int kRate=48000, kChannels=2, kFrames=960;
    aob::OpusPipeline encoder;
    if(!encoder.open(kRate,kChannels,128000)) return false;
    std::array<float,kFrames*kChannels> pcm{};
    for(int i=0;i<kFrames;++i){ float v=0.2f*std::sin(2.0f*3.14159265358979323846f*440.0f*i/kRate); pcm[2*i]=v; pcm[2*i+1]=v; }
    std::array<uint8_t,4096> encoded{};
    auto encoded_size=encoder.encode(pcm,encoded);
    if(!encoded_size) return false;
    std::array<uint8_t,4096> datagram{};
    aob::AudioPacket source{1,7,0,static_cast<uint32_t>(kRate),kChannels,20,{encoded.data(),encoded_size}};
    auto packet_size=aob::serialize_audio(source,datagram);
    if(!packet_size) return false;
    aob::AudioPacket parsed{}; std::span<const uint8_t> payload{};
    if(!aob::deserialize_audio({datagram.data(),packet_size},parsed,payload)) return false;
    int error=OPUS_OK; auto* decoder=opus_decoder_create(kRate,kChannels,&error);
    if(!decoder||error!=OPUS_OK) return false;
    std::array<float,kFrames*kChannels> decoded{};
    int decoded_frames=opus_decode_float(decoder,payload.data(),static_cast<opus_int32>(payload.size()),decoded.data(),kFrames,0);
    opus_decoder_destroy(decoder); encoder.close();
    if(decoded_frames!=kFrames) return false;
    for(float sample:decoded) if(!std::isfinite(sample)||std::abs(sample)>1.0f) return false;
    return true;
}

void status(const std::string& s){ if(gStatus) SetWindowTextA(gStatus,s.c_str()); }

void stop_stream() {
    if(gCapture) gCapture->stop();
    if(gOpus) gOpus->close();
    if(gUdp) gUdp->close();
    gCapture.reset(); gOpus.reset(); gUdp.reset(); gFifo.clear();
    status("Stopped");
}

bool start_stream(const std::string& host,uint16_t port) {
    stop_stream();
    auto udp=std::make_unique<UdpSender>();
    if(!udp->open(host,port)){ status("Error: unable to open UDP transport"); return false; }
    auto opus=std::make_unique<aob::OpusPipeline>();
    if(!opus->open()){ status("Error: Opus encoder failed"); udp->close(); return false; }
    auto cap=std::make_unique<WasapiLoopback>();
    gFifo.reserve(3840);
    if(!cap->start([u=udp.get(),o=opus.get()](const float* pcm,uint32_t frames,uint32_t channels,uint32_t sample_rate){
        if(channels!=2||sample_rate!=48000) return;
        gFifo.insert(gFifo.end(),pcm,pcm+static_cast<size_t>(frames)*2);
        while(gFifo.size()>=1920){
            std::array<uint8_t,4096> encoded{};
            auto n=o->encode({gFifo.data(),1920},encoded);
            if(n){
                std::array<uint8_t,4096> packet{};
                aob::AudioPacket p{1,gSeq.fetch_add(1),gTimestamp.fetch_add(960),48000,2,20,{encoded.data(),n}};
                auto size=aob::serialize_audio(p,packet);
                if(size) u->send({packet.data(),size});
            }
            gFifo.erase(gFifo.begin(),gFifo.begin()+1920);
        }
    })){ status("Error: WASAPI loopback could not start"); opus->close(); udp->close(); gFifo.clear(); return false; }
    gUdp=std::move(udp); gOpus=std::move(opus); gCapture=std::move(cap);
    status("Streaming to "+host+":"+std::to_string(port));
    return true;
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam){
    switch(msg){
    case WM_CREATE:
        CreateWindowA("STATIC","Android IP:",WS_CHILD|WS_VISIBLE,20,22,90,24,hwnd,nullptr,nullptr,nullptr);
        gHost=CreateWindowA("EDIT","",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL,115,18,230,28,hwnd,(HMENU)IDC_HOST,nullptr,nullptr);
        CreateWindowA("STATIC","Port:",WS_CHILD|WS_VISIBLE,365,22,45,24,hwnd,nullptr,nullptr,nullptr);
        gPort=CreateWindowA("EDIT","4677",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_NUMBER,410,18,80,28,hwnd,(HMENU)IDC_PORT,nullptr,nullptr);
        CreateWindowA("BUTTON","Start",WS_CHILD|WS_VISIBLE,20,62,110,32,hwnd,(HMENU)IDC_START,nullptr,nullptr);
        CreateWindowA("BUTTON","Stop",WS_CHILD|WS_VISIBLE,140,62,110,32,hwnd,(HMENU)IDC_STOP,nullptr,nullptr);
        CreateWindowA("BUTTON","Self-test",WS_CHILD|WS_VISIBLE,260,62,110,32,hwnd,(HMENU)IDC_TEST,nullptr,nullptr);
        gStatus=CreateWindowA("STATIC","Ready",WS_CHILD|WS_VISIBLE,20,112,470,48,hwnd,(HMENU)IDC_STATUS,nullptr,nullptr);
        return 0;
    case WM_COMMAND:
        if(LOWORD(wParam)==IDC_START){
            char host[256]{}, portbuf[32]{};
            GetWindowTextA(gHost,host,sizeof(host)); GetWindowTextA(gPort,portbuf,sizeof(portbuf));
            int port=std::atoi(portbuf);
            if(host[0]=='\0'||port<1||port>65535) status("Enter a valid Android IP and UDP port");
            else start_stream(host,static_cast<uint16_t>(port));
        } else if(LOWORD(wParam)==IDC_STOP) stop_stream();
        else if(LOWORD(wParam)==IDC_TEST) status(self_test()?"Self-test: PASS":"Self-test: FAIL");
        return 0;
    case WM_DESTROY: stop_stream(); PostQuitMessage(0); return 0;
    }
    return DefWindowProcA(hwnd,msg,wParam,lParam);
}
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR cmdLine,int nCmdShow){
    if(cmdLine && std::string(cmdLine).find("--self-test")!=std::string::npos) return self_test()?0:10;
    INITCOMMONCONTROLSEX icc{sizeof(icc),ICC_STANDARD_CLASSES}; InitCommonControlsEx(&icc);
    WNDCLASSA wc{}; wc.lpfnWndProc=WndProc; wc.hInstance=hInstance; wc.lpszClassName="AOBSpeakerProWindow"; wc.hCursor=LoadCursor(nullptr,IDC_ARROW); wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    RegisterClassA(&wc);
    HWND hwnd=CreateWindowA(wc.lpszClassName,"AOB Speaker Pro",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,100,100,530,210,nullptr,nullptr,hInstance,nullptr);
    if(!hwnd) return 11;
    ShowWindow(hwnd,nCmdShow); UpdateWindow(hwnd);
    MSG msg{};
    while(GetMessage(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessage(&msg);}
    return static_cast<int>(msg.wParam);
}