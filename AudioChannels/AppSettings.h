#pragma once
#include "framework.h"

// Persistencia simple en el registro de Windows (HKCU) para las opciones
// de configuracion: dispositivo de salida elegido y autoarranque.
namespace AppSettings {

// "" = seguir el dispositivo predeterminado del sistema.
std::wstring GetSelectedDeviceId();
void SetSelectedDeviceId(const std::wstring& deviceId);

bool IsAutoStartEnabled();
void SetAutoStartEnabled(bool enabled);

// "" = automatico (segun el idioma de Windows), "es" o "en" para forzarlo.
std::wstring GetLanguageOverride();
void SetLanguageOverride(const std::wstring& languageCode);

} // namespace AppSettings
