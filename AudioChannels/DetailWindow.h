#pragma once
#include "framework.h"
#include "AudioEngine.h"

// Ventana emergente sin bordes anclada al icono de bandeja, con una barra
// de nivel por canal (nombre, indicador de actividad y dB). Se cierra sola
// al perder el foco, como el flyout de volumen de Windows.
class DetailWindow {
public:
    explicit DetailWindow(HINSTANCE hInstance);
    ~DetailWindow();

    DetailWindow(const DetailWindow&) = delete;
    DetailWindow& operator=(const DetailWindow&) = delete;

    void ShowNear(RECT anchor, const std::vector<AudioEngine::ChannelSnapshot>& channels, const std::wstring& deviceName);
    void Hide();
    void Refresh(const std::vector<AudioEngine::ChannelSnapshot>& channels);
    bool IsVisible() const;

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnPaint(HDC hdc);

    HWND hwnd_ = nullptr;
    HINSTANCE hInstance_;
    std::vector<AudioEngine::ChannelSnapshot> channels_;
    std::wstring deviceName_;

    static constexpr int kWidth = 320;
    static constexpr int kPadding = 12;
    static constexpr int kRowHeight = 34;
    static constexpr int kHeaderHeight = 34;
};
