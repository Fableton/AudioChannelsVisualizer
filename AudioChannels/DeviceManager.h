#pragma once
#include "framework.h"
#include <mmdeviceapi.h>
#include <propsys.h>

struct AudioDeviceInfo {
    std::wstring id;
    std::wstring name;
};

// Escucha los cambios del dispositivo de salida predeterminado de Windows
// (IMMNotificationClient) y avisa via callback para que el motor de audio
// se reconfigure solo. Tambien permite enumerar los dispositivos de salida
// disponibles para la pantalla de configuracion.
class DeviceManager : public IMMNotificationClient {
public:
    DeviceManager();
    ~DeviceManager();

    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;

    void SetOnDefaultDeviceChanged(std::function<void()> callback);
    std::vector<AudioDeviceInfo> EnumerateRenderDevices() const;

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    // IMMNotificationClient
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override;

private:
    std::atomic<ULONG> refCount_{ 1 };
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator_;
    std::function<void()> onDefaultDeviceChanged_;
};
