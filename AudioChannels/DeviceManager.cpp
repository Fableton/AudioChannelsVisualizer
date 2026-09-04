#include "DeviceManager.h"

// PKEY_Device_FriendlyName solo se necesita aqui; INITGUID hace que el
// header defina el dato real en vez de declararlo extern (evita depender
// de Propsys.lib). No incluir este header con INITGUID en otro .cpp.
#define INITGUID
#include <Functiondiscoverykeys_devpkey.h>

using Microsoft::WRL::ComPtr;

DeviceManager::DeviceManager() {
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(enumerator_.ReleaseAndGetAddressOf()));
    if (SUCCEEDED(hr) && enumerator_) {
        enumerator_->RegisterEndpointNotificationCallback(this);
    }
}

DeviceManager::~DeviceManager() {
    if (enumerator_) {
        enumerator_->UnregisterEndpointNotificationCallback(this);
    }
}

void DeviceManager::SetOnDefaultDeviceChanged(std::function<void()> callback) {
    onDefaultDeviceChanged_ = std::move(callback);
}

HRESULT DeviceManager::QueryInterface(REFIID riid, void** ppvObject) {
    if (!ppvObject) return E_POINTER;
    if (riid == __uuidof(IUnknown) || riid == __uuidof(IMMNotificationClient)) {
        *ppvObject = static_cast<IMMNotificationClient*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG DeviceManager::AddRef() {
    return ++refCount_;
}

ULONG DeviceManager::Release() {
    // El ciclo de vida de este objeto lo controla su dueno (no COM);
    // solo llevamos la cuenta, nunca nos autodestruimos aqui.
    return --refCount_;
}

HRESULT DeviceManager::OnDeviceStateChanged(LPCWSTR, DWORD) { return S_OK; }
HRESULT DeviceManager::OnDeviceAdded(LPCWSTR) { return S_OK; }
HRESULT DeviceManager::OnDeviceRemoved(LPCWSTR) { return S_OK; }

HRESULT DeviceManager::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR) {
    if (flow == eRender && role == eConsole && onDefaultDeviceChanged_) {
        onDefaultDeviceChanged_();
    }
    return S_OK;
}

HRESULT DeviceManager::OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) { return S_OK; }
