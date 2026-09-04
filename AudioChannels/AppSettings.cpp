#include "AppSettings.h"

namespace AppSettings {
namespace {

constexpr wchar_t kSettingsKeyPath[] = L"Software\\Fableton\\AudioChannels";
constexpr wchar_t kDeviceIdValueName[] = L"SelectedDeviceId";
constexpr wchar_t kLanguageValueName[] = L"Language";

constexpr wchar_t kRunKeyPath[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kRunValueName[] = L"AudioChannels";

std::wstring GetExecutablePath() {
    wchar_t path[MAX_PATH]{};
    DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::wstring(path, len);
}

std::wstring ReadStringValue(const wchar_t* valueName) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kSettingsKeyPath, 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return L"";
    }

    wchar_t buffer[512]{};
    DWORD size = sizeof(buffer);
    DWORD type = 0;
    LONG result = RegQueryValueExW(key, valueName, nullptr, &type, reinterpret_cast<BYTE*>(buffer), &size);
    RegCloseKey(key);

    if (result != ERROR_SUCCESS || type != REG_SZ) return L"";
    return std::wstring(buffer);
}

void WriteStringValue(const wchar_t* valueName, const std::wstring& value) {
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kSettingsKeyPath, 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return;
    }
    RegSetValueExW(key, valueName, 0, REG_SZ,
        reinterpret_cast<const BYTE*>(value.c_str()),
        static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(key);
}

} // namespace

std::wstring GetSelectedDeviceId() {
    return ReadStringValue(kDeviceIdValueName);
}

void SetSelectedDeviceId(const std::wstring& deviceId) {
    WriteStringValue(kDeviceIdValueName, deviceId);
}

std::wstring GetLanguageOverride() {
    return ReadStringValue(kLanguageValueName);
}

void SetLanguageOverride(const std::wstring& languageCode) {
    WriteStringValue(kLanguageValueName, languageCode);
}

bool IsAutoStartEnabled() {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return false;
    }
    DWORD type = 0;
    LONG result = RegQueryValueExW(key, kRunValueName, nullptr, &type, nullptr, nullptr);
    RegCloseKey(key);
    return result == ERROR_SUCCESS && type == REG_SZ;
}

void SetAutoStartEnabled(bool enabled) {
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return;
    }
    if (enabled) {
        std::wstring quotedPath = L"\"" + GetExecutablePath() + L"\"";
        RegSetValueExW(key, kRunValueName, 0, REG_SZ,
            reinterpret_cast<const BYTE*>(quotedPath.c_str()),
            static_cast<DWORD>((quotedPath.size() + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(key, kRunValueName);
    }
    RegCloseKey(key);
}

} // namespace AppSettings
