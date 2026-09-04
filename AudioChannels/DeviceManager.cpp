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

std::vector<AudioDeviceInfo> DeviceManager::EnumerateRenderDevices() const {
    std::vector<AudioDeviceInfo> result;
    if (!enumerator_) return result;

    ComPtr<IMMDeviceCollection> collection;
    if (FAILED(enumerator_->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, collection.ReleaseAndGetAddressOf()))) {
        return result;
    }

    UINT count = 0;
    collection->GetCount(&count);
    for (UINT i = 0; i < count; ++i) {
        ComPtr<IMMDevice> device;
        if (FAILED(collection->Item(i, device.ReleaseAndGetAddressOf()))) continue;

        LPWSTR idStr = nullptr;
        if (FAILED(device->GetId(&idStr))) continue;
        std::wstring id = idStr;
        CoTaskMemFree(idStr);

        std::wstring name = id;
        ComPtr<IPropertyStore> props;
        if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, props.ReleaseAndGetAddressOf()))) {
            PROPVARIANT var;
            PropVariantInit(&var);
            if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &var)) && var.vt == VT_LPWSTR && var.pwszVal) {
                name = var.pwszVal;
            }
            PropVariantClear(&var);
        }

        result.push_back({ std::move(id), std::move(name) });
    }
    return result;
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
