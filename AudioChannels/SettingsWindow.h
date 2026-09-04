#pragma once
#include "framework.h"
#include "DeviceManager.h"

// Ventana de configuracion: elegir el dispositivo de salida a monitorear
// (o "predeterminado") y habilitar/deshabilitar el autoarranque con
// Windows. Guarda en el registro via AppSettings al confirmar.
//
// Es Win32 nativo (no hay WinForms/TableLayoutPanel aca), asi que el
// layout se recalcula a mano en Layout(): las medidas se definen en
// pixeles logicos a 96 DPI y se escalan al DPI real de la ventana, y se
// vuelve a ejecutar en cada WM_SIZE/WM_DPICHANGED para que la ventana sea
// redimensionable y se vea bien en pantallas de alto DPI.
class SettingsWindow {
public:
    SettingsWindow(HINSTANCE hInstance, HWND owner, DeviceManager& deviceManager);
    ~SettingsWindow();

    SettingsWindow(const SettingsWindow&) = delete;
    SettingsWindow& operator=(const SettingsWindow&) = delete;

    // deviceId == "" significa "predeterminado del sistema".
    void SetOnApply(std::function<void(const std::wstring& deviceId)> callback);

    void Show();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void CreateControls(HWND hwnd);
    void PopulateDeviceList();
    void OnOk();
    void RecreateFont();
    void Layout();

    HWND hwnd_ = nullptr;
    HWND owner_;
    HINSTANCE hInstance_;
    DeviceManager& deviceManager_;

    HWND deviceLabel_ = nullptr;
    HWND deviceCombo_ = nullptr;
    HWND autoStartCheck_ = nullptr;
    HWND okBtn_ = nullptr;
    HWND cancelBtn_ = nullptr;

    HFONT font_ = nullptr;
    UINT dpi_ = 96;

    // Paralelo a los items del combo: indice 0 = predeterminado (id = "").
    std::vector<std::wstring> deviceIds_;
    std::function<void(const std::wstring&)> onApply_;
};
