#include "framework.h"
#include "DeviceManager.h"
#include "AudioEngine.h"
#include "TrayIcon.h"
#include "DetailWindow.h"
#include "SettingsWindow.h"
#include "AppSettings.h"
#include "Localization.h"

namespace {

constexpr UINT WM_TRAYICON = WM_APP + 1;
constexpr UINT_PTR kRefreshTimerId = 1;
constexpr UINT kRefreshIntervalMs = 66; // ~15 Hz, varias actualizaciones por segundo
constexpr wchar_t kMainClassName[] = L"AudioChannelsMainWnd";
constexpr wchar_t kSingleInstanceMutexName[] = L"Fableton_AudioChannels_SingleInstance_9F3A2B10";
constexpr UINT_PTR ID_MENU_SETTINGS = 1;
constexpr UINT_PTR ID_MENU_EXIT = 2;

std::unique_ptr<DeviceManager> g_deviceManager;
std::unique_ptr<AudioEngine> g_audioEngine;
std::unique_ptr<TrayIcon> g_trayIcon;
std::unique_ptr<DetailWindow> g_detailWindow;
std::unique_ptr<SettingsWindow> g_settingsWindow;

void RefreshUI() {
    auto channels = g_audioEngine->GetChannels();
    auto deviceName = g_audioEngine->GetDeviceFriendlyName();
    if (g_trayIcon) g_trayIcon->Update(channels, deviceName);
    if (g_detailWindow && g_detailWindow->IsVisible()) g_detailWindow->Refresh(channels);
}

void ToggleDetailWindow() {
    if (!g_detailWindow) return;
    if (g_detailWindow->IsVisible()) {
        g_detailWindow->Hide();
    } else {
        RECT anchor = g_trayIcon->GetIconRect();
        g_detailWindow->ShowNear(anchor, g_audioEngine->GetChannels(), g_audioEngine->GetDeviceFriendlyName());
    }
}

void ShowTrayMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, ID_MENU_SETTINGS, Loc::T(Loc::Str::TrayMenuSettings));
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_MENU_EXIT, Loc::T(Loc::Str::TrayMenuExit));
    SetForegroundWindow(hwnd); // requerido para que el menu se cierre al perder foco
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TRAYICON:
        if (LOWORD(lParam) == WM_LBUTTONUP) {
            ToggleDetailWindow();
        } else if (LOWORD(lParam) == WM_RBUTTONUP) {
            ShowTrayMenu(hwnd);
        }
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_MENU_SETTINGS) {
            if (g_settingsWindow) g_settingsWindow->Show();
        } else if (LOWORD(wParam) == ID_MENU_EXIT) {
            DestroyWindow(hwnd);
        }
        return 0;
    case WM_TIMER:
        if (wParam == kRefreshTimerId) RefreshUI();
        return 0;
    case WM_DESTROY:
        KillTimer(hwnd, kRefreshTimerId);
        if (g_trayIcon) g_trayIcon->Hide();
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

} // namespace

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    Loc::Init();

    HANDLE singleInstanceMutex = CreateMutexW(nullptr, TRUE, kSingleInstanceMutexName);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(nullptr, Loc::T(Loc::Str::AlreadyRunningMessage), Loc::T(Loc::Str::AlreadyRunningTitle), MB_OK | MB_ICONINFORMATION);
        if (singleInstanceMutex) CloseHandle(singleInstanceMutex);
        return 0;
    }

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kMainClassName;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, kMainClassName, L"AudioChannels", 0,
        0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

    g_deviceManager = std::make_unique<DeviceManager>();
    g_audioEngine = std::make_unique<AudioEngine>();
    g_trayIcon = std::make_unique<TrayIcon>(hwnd, WM_TRAYICON);
    g_detailWindow = std::make_unique<DetailWindow>(hInstance);
    g_settingsWindow = std::make_unique<SettingsWindow>(hInstance, hwnd, *g_deviceManager);

    g_deviceManager->SetOnDefaultDeviceChanged([]() {
        if (g_audioEngine && g_audioEngine->IsFollowingDefaultDevice()) {
            g_audioEngine->RestartCapture();
        }
    });

    g_settingsWindow->SetOnApply([](const std::wstring& deviceId) {
        if (g_audioEngine) g_audioEngine->UseDevice(deviceId);
    });

    // Fija el dispositivo guardado antes de Start(): el hilo de captura lo
    // toma directo en su primera vuelta, sin un ciclo extra de reconexion.
    g_audioEngine->UseDevice(AppSettings::GetSelectedDeviceId());
    g_audioEngine->Start();
    g_trayIcon->Show();
    SetTimer(hwnd, kRefreshTimerId, kRefreshIntervalMs, nullptr);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    g_audioEngine->Stop();
    g_settingsWindow.reset();
    g_detailWindow.reset();
    g_trayIcon.reset();
    g_audioEngine.reset();
    g_deviceManager.reset();

    CoUninitialize();
    if (singleInstanceMutex) CloseHandle(singleInstanceMutex);
    return static_cast<int>(msg.wParam);
}
