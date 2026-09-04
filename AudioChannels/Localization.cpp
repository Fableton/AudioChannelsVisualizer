#include "Localization.h"
#include "AppSettings.h"
#include <winnls.h>

namespace Loc {
namespace {

Language g_current = Language::Spanish;

struct Entry {
    const wchar_t* es;
    const wchar_t* en;
};

// Debe mantenerse en el mismo orden que el enum Str.
const Entry kTable[] = {
    /* TrayMenuSettings        */ { L"Configuracion...", L"Settings..." },
    /* TrayMenuExit            */ { L"Salir", L"Exit" },
    /* AlreadyRunningTitle     */ { L"AudioChannels", L"AudioChannels" },
    /* AlreadyRunningMessage   */ { L"AudioChannels ya se esta ejecutando.", L"AudioChannels is already running." },
    /* SettingsWindowTitle     */ { L"Configuracion de AudioChannels", L"AudioChannels Settings" },
    /* SettingsDeviceLabel     */ { L"Dispositivo de salida a monitorear:", L"Output device to monitor:" },
    /* SettingsDeviceDefault   */ { L"Predeterminado (el del sistema)", L"Default (system device)" },
    /* SettingsAutoStartLabel  */ { L"Iniciar automaticamente con Windows", L"Start automatically with Windows" },
    /* SettingsLanguageLabel   */ { L"Idioma:", L"Language:" },
    /* SettingsLanguageAuto    */ { L"Automatico (segun Windows)", L"Automatic (matches Windows)" },
    /* SettingsLanguageSpanish */ { L"Espanol", L"Spanish" },
    /* SettingsLanguageEnglish */ { L"English", L"English" },
    /* SettingsSaveButton      */ { L"Guardar", L"Save" },
    /* SettingsCancelButton    */ { L"Cancelar", L"Cancel" },
};

} // namespace

Language DetectSystemLanguage() {
    LANGID langId = GetUserDefaultUILanguage();
    return (PRIMARYLANGID(langId) == LANG_ENGLISH) ? Language::English : Language::Spanish;
}

void SetLanguage(Language language) {
    g_current = language;
}

Language CurrentLanguage() {
    return g_current;
}

void Init() {
    std::wstring override = AppSettings::GetLanguageOverride();
    if (override == L"en") {
        g_current = Language::English;
    } else if (override == L"es") {
        g_current = Language::Spanish;
    } else {
        g_current = DetectSystemLanguage();
    }
}

const wchar_t* T(Str id) {
    const Entry& entry = kTable[static_cast<size_t>(id)];
    return (g_current == Language::English) ? entry.en : entry.es;
}

std::wstring ChannelFallbackName(int oneBasedIndex) {
    std::wstring idx = std::to_wstring(oneBasedIndex);
    return (g_current == Language::English ? L"Channel " : L"Canal ") + idx;
}

std::wstring ChannelCountLabel(size_t count) {
    std::wstring n = std::to_wstring(count);
    return n + (g_current == Language::English ? L" channels" : L" canales");
}

} // namespace Loc
