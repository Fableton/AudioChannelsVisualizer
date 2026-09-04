#pragma once
#include "framework.h"

// Textos de la interfaz en espanol/ingles. Los nombres de canal (Front
// Left, LFE, etc.) NO se traducen: son terminologia tecnica estandar de
// audio, igual en cualquier idioma (ver ChannelLayout).
namespace Loc {

enum class Language { Spanish, English };

enum class Str {
    TrayMenuSettings,
    TrayMenuExit,
    AlreadyRunningTitle,
    AlreadyRunningMessage,
    SettingsWindowTitle,
    SettingsDeviceLabel,
    SettingsDeviceDefault,
    SettingsAutoStartLabel,
    SettingsLanguageLabel,
    SettingsLanguageAuto,
    SettingsLanguageSpanish,
    SettingsLanguageEnglish,
    SettingsSaveButton,
    SettingsCancelButton,
};

// Idioma de la UI de Windows: ingles si esta en ingles, espanol para
// cualquier otro caso (idioma de referencia de la app).
Language DetectSystemLanguage();

// Resuelve el idioma activo a partir de AppSettings::GetLanguageOverride()
// ("" = automatico segun Windows). Se llama una vez al arrancar, antes de
// crear cualquier ventana.
void Init();

void SetLanguage(Language language);
Language CurrentLanguage();

const wchar_t* T(Str id);

std::wstring ChannelFallbackName(int oneBasedIndex); // "Canal 3" / "Channel 3"
std::wstring ChannelCountLabel(size_t count);         // "3 canales" / "3 channels"

} // namespace Loc
