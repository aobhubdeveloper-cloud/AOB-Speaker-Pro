#include "wasapi_loopback.hpp"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <thread>
#include <vector>

bool WasapiLoopback::start(Callback cb) {
    if (running_) return false;
    running_ = true;

    worker_ = new std::thread([this, cb] {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        IMMDeviceEnumerator* en = nullptr;
        IMMDevice* d = nullptr;
        IAudioClient* c = nullptr;
        IAudioCaptureClient* cap = nullptr;
        WAVEFORMATEX* f = nullptr;
        HANDLE ev = nullptr;

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

        while (running_) {
            WaitForSingleObject(ev, 50);

            UINT32 n = 0;
            if (FAILED(cap->GetNextPacketSize(&n))) break;

            while (n) {
                BYTE* p = nullptr;
                UINT32 frames = n;
                DWORD flags = 0;

                // Windows 11 SDK exposes device-position and QPC-position outputs.
                if (FAILED(cap->GetBuffer(&p, &frames, &flags, nullptr, nullptr))) break;

                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    static thread_local std::vector<float> z;
                    z.assign(size_t(frames) * f->nChannels, 0.0f);
                    cb(z.data(), frames, f->nChannels);
                } else {
                    cb(reinterpret_cast<const float*>(p), frames, f->nChannels);
                }

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
    });

    return true;
}

void WasapiLoopback::stop() {
    running_ = false;
    if (worker_) {
        auto* t = static_cast<std::thread*>(worker_);
        t->join();
        delete t;
        worker_ = nullptr;
    }
}
