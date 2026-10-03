#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <thread>

class WasapiLoopback {
public:
    using Callback = std::function<void(const float*, uint32_t, uint32_t, uint32_t)>;

    bool start(Callback);
    void stop();

private:
    std::thread* worker_ = nullptr;
    std::atomic<bool> running_{false};
};
