#pragma once
#include "framework.h"
#include "AudioEngine.h"

// Dibuja y mantiene el icono de la bandeja del sistema: un punto por canal,
// verde si tiene senal, gris si no. Se reconstruye a cada refresco de UI.
class TrayIcon {
public:
    TrayIcon(HWND hwnd, UINT callbackMessage);
    ~TrayIcon();

    TrayIcon(const TrayIcon&) = delete;
    TrayIcon& operator=(const TrayIcon&) = delete;

    void Show();
    void Hide();
    void Update(const std::vector<AudioEngine::ChannelSnapshot>& channels, const std::wstring& deviceName);

    // Rectangulo en pantalla del icono, para anclar la ventana de detalle.
    RECT GetIconRect() const;

private:
    HICON BuildIcon(const std::vector<AudioEngine::ChannelSnapshot>& channels) const;

    HWND hwnd_;
    UINT callbackMessage_;
    NOTIFYICONDATAW nid_{};
    HICON currentIcon_ = nullptr;
    bool visible_ = false;
};
