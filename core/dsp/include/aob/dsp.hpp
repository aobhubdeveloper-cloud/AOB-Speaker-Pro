#pragma once
#include <array>
#include <cstddef>
namespace aob{
enum class Preset{Original,Balanced,Music,Movie,Gaming,Voice,BassBoost,Loudness,AobMax,Custom};
struct DspSettings{bool enabled=true;float preamp_db=0;std::array<float,10>eq_db{};float bass_db=0,treble_db=0,loudness=0,compressor=0,limiter_db=-1,output_db=0;};
class DspEngine{DspSettings s_{};public:void configure(const DspSettings&s){s_=s;}void set_preset(Preset);void process(float*,size_t,size_t);const DspSettings&settings()const{return s_;}};
}