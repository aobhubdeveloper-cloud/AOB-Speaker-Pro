#pragma once
#include <cstdint>
namespace aob{struct Telemetry{uint64_t received=0,lost=0,reordered=0,underruns=0;float jitter_ms=0,buffer_ms=0,drift_ppm=0,rtt_ms=0;};inline float loss_percent(const Telemetry&t){auto n=t.received+t.lost;return n?100.f*float(t.lost)/float(n):0.f;}}