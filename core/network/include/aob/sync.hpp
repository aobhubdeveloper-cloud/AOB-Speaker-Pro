#pragma once
#include <cstdint>
namespace aob{struct SyncState{uint64_t master_timestamp=0;uint64_t local_timestamp=0;double rate=1.0;};inline double correction_ppm(const SyncState&s){return (s.rate-1.0)*1000000.0;}}