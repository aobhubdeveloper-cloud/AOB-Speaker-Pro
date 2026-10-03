#include "wasapi_loopback.hpp"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <ksmedia.h>
#include <algorithm>
#include <cmath>
#include <thread>
#include <vector>

namespace {
bool is_float_format(const WAVEFORMATEX* f) {
    if (f->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) return true;
    if (f->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        const auto* e = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(f);
        return IsEqualGUID(e->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
    }
    return false;
}

void convert_to_float(const BYTE* input, uint32_t frames, const WAVEFORMATEX* f, std::vector<float>& out) {
    const uint32_t channels = f->nChannels;
    out.resize(static_cast<size_t>(frames) * channels);

    if (is_float_format(f) && f->wBitsPerSample == 32) {
        const auto* src = reinterpret_cast<const float*>(input);
        std::copy(src, src + out.size(), out.begin());
        return;
    }

    if (f->wBitsPerSample == 16) {
        const auto* src = reinterpret_cast<const int16_t*>(input);
        for (size_t i = 0; i < out.size(); ++i) out[i] = static_cast<float>(src[i]) / 32768.0f;
        return;
    }

    if (f->wBitsPerSample == 24) {
        for (size_t i = 0; i < out.size(); ++i) {
            const auto* p = input + i * 3;
            int32_t v = (static_cast<int32_t>(p[0]) |
                         (static_cast<int32_t>(p[1]) << 8) |
                         (static_cast<int32_t>(p[2]) << 16));
            if (v & 0x00800000) v |= 0xFF000000;
            out[i] = static_cast<float>(v) / 8388608.0f;
        }
        return;
    }

    if (f->wBitsPerSample == 32) {
        const auto* src = reinterpret_cast<const int32_t*>(input);
        for (size_t i = 0; i < out.size(); ++i) out[i] = static_cast<float>(src[i]) / 2147483648.0f;
        return;
    }

    std::fill(out.begin(), out.end(), 0.0f);
}
}

bool WasapiLoopback::start(Callback cb) {
    if (running_.exchange(true)) return false;

    worker_ = new std::thread([this, cb] {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        IMMDeviceEnumerator* en = nullptr;
        IMMDevice* d = nullptr;
        IAudioClient* c = nullptr;
        IAudioCaptureClient* cap = nullptr;
        WAVEFORMATEX* f = nullptr;
        HANDLE ev = nullptr;
        std::vector<float> converted;

        if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                    IID_PPV_ARGS(&en))) ||
            FAILED(en->GetDefaultAudioEndpoint(eRender, eConsole, &d)) ||
            FAILED(d->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                               reinterpret_cast<void**>(&c)))) {
            goto done;
        }

        if (FAILED(c->GetMixFormat(&f))) goto done;

        if (FAILED(c->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                  AUDCLNT_STREAMFLAGS_LOOPBACK |
                                      AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                  200000, 0, f, nullptr))) {
            goto done;
        }

        if (FAILED(c->GetService(IID_PPV_ARGS(&cap)))) goto done;

        ev = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!ev || FAILED(c->SetEventHandle(ev)) || FAILED(c->Start())) goto done;

        while (running_.load(std::memory_order_acquire)) {
            WaitForSingleObject(ev, 50);

            UINT32 n = 0;
            if (FAILED(cap->GetNextPacketSize(&n))) break;

            while (n && running_.load(std::memory_order_acquire)) {
                BYTE* p = nullptr;
                UINT32 frames = n;
                DWORD flags = 0;

                if (FAILED(cap->GetBuffer(&p, &frames, &flags, nullptr, nullptr))) break;

                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    converted.assign(static_cast<size_t>(frames) * f->nChannels, 0.0f);
                } else {
                    convert_to_float(p, frames, f, converted);
                }

                cb(converted.data(), frames, f->nChannels, f->nSamplesPerSec);
                cap->ReleaseBuffer(frames);

                if (FAILED(cap->GetNextPacketSize(&n))) break;
            }
        }

    done:
        if (c) c->Stop();
        if (ev) CloseHandle(ev);
        if (cap) cap->Release();
        if (c) c->Release();
        if (d) d->Release();
        if (en) en->Release();
        if (f) CoTaskMemFree(f);
        CoUninitialize();
        running_.store(false, std::memory_order_release);
    });

    return true;
}

void WasapiLoopback::stop() {
    running_.store(false, std::memory_order_release);
    if (worker_) {
        worker_->join();
        delete worker_;
        worker_ = nullptr;
    }
}
