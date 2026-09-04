#include "AudioEngine.h"
#include "ChannelLayout.h"
#include <Functiondiscoverykeys_devpkey.h>

using Microsoft::WRL::ComPtr;

namespace {
constexpr float kSilenceFloorDb = -100.0f;
constexpr REFERENCE_TIME kBufferDuration = 200000; // 20 ms, en unidades de 100 ns
}

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine() {
    Stop();
}

void AudioEngine::Start() {
    if (running_.exchange(true)) return;
    stopEvent_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    restartEvent_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    dataEvent_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    thread_ = std::thread(&AudioEngine::ThreadProc, this);
}

void AudioEngine::Stop() {
    if (!running_.exchange(false)) return;
    if (stopEvent_) SetEvent(stopEvent_);
    if (thread_.joinable()) thread_.join();
    if (stopEvent_) { CloseHandle(stopEvent_); stopEvent_ = nullptr; }
    if (restartEvent_) { CloseHandle(restartEvent_); restartEvent_ = nullptr; }
    if (dataEvent_) { CloseHandle(dataEvent_); dataEvent_ = nullptr; }
}

void AudioEngine::RestartWithDefaultDevice() {
    if (running_.load() && restartEvent_) SetEvent(restartEvent_);
}

std::vector<AudioEngine::ChannelSnapshot> AudioEngine::GetChannels() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return channels_;
}

std::wstring AudioEngine::GetDeviceFriendlyName() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return deviceFriendlyName_;
}

void AudioEngine::ThreadProc() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    while (running_.load(std::memory_order_acquire)) {
        if (!InitializeForDefaultDevice()) {
            CleanupDeviceResources();
            HANDLE waits[] = { stopEvent_, restartEvent_ };
            WaitForMultipleObjects(2, waits, FALSE, 1000);
            continue;
        }

        HRESULT hr = audioClient_->Start();
        if (FAILED(hr)) {
            CleanupDeviceResources();
            HANDLE waits[] = { stopEvent_, restartEvent_ };
            WaitForMultipleObjects(2, waits, FALSE, 1000);
            continue;
        }

        bool restart = false;
        while (running_.load(std::memory_order_acquire) && !restart) {
            HANDLE waits[] = { stopEvent_, restartEvent_, dataEvent_ };
            DWORD wait = WaitForMultipleObjects(3, waits, FALSE, 500);
            if (wait == WAIT_OBJECT_0) {
                running_.store(false, std::memory_order_release);
                break;
            } else if (wait == WAIT_OBJECT_0 + 1) {
                restart = true;
            } else if (wait == WAIT_OBJECT_0 + 2) {
                DrainPackets();
            }
            // WAIT_TIMEOUT: seguimos esperando (no hubo paquete nuevo).
        }

        audioClient_->Stop();
        CleanupDeviceResources();
    }

    CoUninitialize();
}

bool AudioEngine::InitializeForDefaultDevice() {
    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(enumerator.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return false;

    ComPtr<IMMDevice> device;
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return false; // sin dispositivo de salida activo

    std::wstring friendlyName;
    ComPtr<IPropertyStore> props;
    if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, props.ReleaseAndGetAddressOf()))) {
        PROPVARIANT var;
        PropVariantInit(&var);
        if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &var)) && var.vt == VT_LPWSTR && var.pwszVal) {
            friendlyName = var.pwszVal;
        }
        PropVariantClear(&var);
    }

    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
        reinterpret_cast<void**>(audioClient_.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return false;

    WAVEFORMATEX* mixFormat = nullptr;
    hr = audioClient_->GetMixFormat(&mixFormat);
    if (FAILED(hr) || !mixFormat) return false;

    channelCount_ = mixFormat->nChannels;
    // El "mix format" del motor de audio compartido de Windows siempre es
    // float de 32 bits; el branch de 16 bits queda como resguardo.
    isFloatFormat_ = (mixFormat->wBitsPerSample == 32);

    DWORD channelMask = 0;
    if (mixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        auto* ext = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mixFormat);
        channelMask = ext->dwChannelMask;
    }

    hr = audioClient_->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        kBufferDuration, 0, mixFormat, nullptr);
    CoTaskMemFree(mixFormat);
    if (FAILED(hr)) return false;

    hr = audioClient_->SetEventHandle(dataEvent_);
    if (FAILED(hr)) return false;

    hr = audioClient_->GetService(__uuidof(IAudioCaptureClient),
        reinterpret_cast<void**>(captureClient_.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return false;

    auto layout = ChannelLayout::Resolve(channelCount_, channelMask);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        deviceFriendlyName_ = friendlyName;
        channels_.clear();
        channels_.reserve(layout.size());
        for (auto& def : layout) {
            ChannelSnapshot snap;
            snap.name = def.name;
            snap.shortName = def.shortName;
            snap.x = def.x;
            snap.y = def.y;
            snap.active = false;
            snap.levelDb = kSilenceFloorDb;
            channels_.push_back(std::move(snap));
        }
    }
    return true;
}

void AudioEngine::CleanupDeviceResources() {
    captureClient_.Reset();
    audioClient_.Reset();
}

void AudioEngine::DrainPackets() {
    UINT32 packetLength = 0;
    HRESULT hr = captureClient_->GetNextPacketSize(&packetLength);
    if (FAILED(hr)) return;

    std::vector<float> peaks(channelCount_, 0.0f);
    bool gotAny = false;

    while (packetLength != 0) {
        BYTE* data = nullptr;
        UINT32 numFrames = 0;
        DWORD flags = 0;
        hr = captureClient_->GetBuffer(&data, &numFrames, &flags, nullptr, nullptr);
        if (FAILED(hr)) break;
        gotAny = true;

        if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT) && data != nullptr) {
            if (isFloatFormat_) {
                const float* samples = reinterpret_cast<const float*>(data);
                for (UINT32 f = 0; f < numFrames; ++f) {
                    for (UINT32 c = 0; c < channelCount_; ++c) {
                        float v = fabsf(samples[static_cast<size_t>(f) * channelCount_ + c]);
                        if (v > peaks[c]) peaks[c] = v;
                    }
                }
            } else {
                const int16_t* samples = reinterpret_cast<const int16_t*>(data);
                for (UINT32 f = 0; f < numFrames; ++f) {
                    for (UINT32 c = 0; c < channelCount_; ++c) {
                        float v = fabsf(samples[static_cast<size_t>(f) * channelCount_ + c] / 32768.0f);
                        if (v > peaks[c]) peaks[c] = v;
                    }
                }
            }
        }

        captureClient_->ReleaseBuffer(numFrames);
        hr = captureClient_->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) break;
    }

    if (!gotAny) return;

    std::lock_guard<std::mutex> lock(mutex_);
    for (size_t c = 0; c < channels_.size() && c < peaks.size(); ++c) {
        channels_[c].active = peaks[c] > 0.0f;
        channels_[c].levelDb = peaks[c] > 0.0f ? 20.0f * log10f(peaks[c]) : kSilenceFloorDb;
    }
}
