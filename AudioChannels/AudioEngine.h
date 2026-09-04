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

    // Cambia el dispositivo a monitorear ("" = predeterminado del sistema)
    // y fuerza al hilo de captura a reinicializarse con el nuevo. Se puede
    // llamar antes de Start() para fijar el dispositivo inicial.
    void UseDevice(const std::wstring& deviceId);
    bool IsFollowingDefaultDevice() const;

    // Se llama cuando el dispositivo de salida predeterminado cambio en
    // Windows; solo tiene efecto si UseDevice() no fijo uno especifico.
    void RestartCapture();

    std::vector<ChannelSnapshot> GetChannels() const;
    std::wstring GetDeviceFriendlyName() const;

private:
    void ThreadProc();
    bool InitializeCaptureDevice();
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
    std::vector<std::chrono::steady_clock::time_point> lastActiveAt_; // en paralelo a channels_
    std::wstring deviceFriendlyName_;
    std::wstring requestedDeviceId_; // "" = seguir el dispositivo predeterminado
};
