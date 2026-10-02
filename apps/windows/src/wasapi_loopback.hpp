#pragma once
#include <cstdint>
#include <functional>
class WasapiLoopback{public:using Callback=std::function<void(const float*,uint32_t,uint32_t)>;bool start(Callback);void stop();private:void*worker_=nullptr;bool running_=false;};