#pragma once
#include "framework.h"
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <propsys.h>

// Captura loopback WASAPI del dispositivo de salida predeterminado en un
// hilo dedicado, y publica un snapshot por canal (activo + nivel en dB)
// thread-safe para que la UI lo lea a su propio ritmo de refresco.
class AudioEngine {
public:
    struct ChannelSnapshot {
        std::wstring name;
        std::wstring shortName;
        float x = 0.5f;
        float y = 0.5f;
        bool active = false;
        float levelDb = -100.0f;
    };

    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    void Start();
    void Stop();

    // Se llama cuando el dispositivo de salida predeterminado cambio en
    // Windows; fuerza al hilo de captura a reinicializarse con el nuevo.
    void RestartWithDefaultDevice();

    std::vector<ChannelSnapshot> GetChannels() const;
    std::wstring GetDeviceFriendlyName() const;

private:
    void ThreadProc();
    bool InitializeForDefaultDevice();
    void DrainPackets();
    void CleanupDeviceResources();

    std::thread thread_;
    std::atomic<bool> running_{ false };
    HANDLE stopEvent_ = nullptr;
    HANDLE restartEvent_ = nullptr;
    HANDLE dataEvent_ = nullptr;

    Microsoft::WRL::ComPtr<IAudioClient> audioClient_;
    Microsoft::WRL::ComPtr<IAudioCaptureClient> captureClient_;
    UINT32 channelCount_ = 0;
    bool isFloatFormat_ = true;

    mutable std::mutex mutex_;
    std::vector<ChannelSnapshot> channels_;
    std::wstring deviceFriendlyName_;
};
