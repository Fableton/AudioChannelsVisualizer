#include "SettingsWindow.h"
#include "AppSettings.h"

namespace {
constexpr wchar_t kClassName[] = L"AudioChannelsSettingsWnd";
constexpr int IDC_DEVICE_COMBO = 101;
constexpr int IDC_AUTOSTART_CHECK = 102;
constexpr int IDC_OK = 103;
constexpr int IDC_CANCEL = 104;

// Medidas logicas en pixeles a 96 DPI (100%); Layout() las escala al DPI
// real de la ventana con Scale().
constexpr int kLogicalWidth = 380;
constexpr int kLogicalHeight = 190;
constexpr int kMinLogicalWidth = 300;
constexpr int kMinLogicalHeight = 170;
constexpr int kMargin = 16;
constexpr int kLabelHeight = 18;
constexpr int kLabelGap = 4;
constexpr int kComboClosedHeight = 23;
constexpr int kComboDropHeight = 200;
constexpr int kCheckHeight = 22;
constexpr int kRowGap = 12;
constexpr int kButtonWidth = 90;
constexpr int kButtonHeight = 26;
constexpr int kButtonGap = 8;
constexpr int kMinContentWidth = 120;

int Scale(int value, UINT dpi) {
    return MulDiv(value, static_cast<int>(dpi), 96);
}
}

SettingsWindow::SettingsWindow(HINSTANCE hInstance, HWND owner, DeviceManager& deviceManager)
    : owner_(owner), hInstance_(hInstance), deviceManager_(deviceManager) {
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance_;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<INT_PTR>(COLOR_BTNFACE + 1));
        wc.lpszClassName = kClassName;
        RegisterClassExW(&wc);
        classRegistered = true;
    }

    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX;
    DWORD exStyle = WS_EX_DLGMODALFRAME;

    hwnd_ = CreateWindowExW(exStyle, kClassName, L"Configuracion de AudioChannels",
        style, CW_USEDEFAULT, CW_USEDEFAULT, kLogicalWidth, kLogicalHeight,
        owner_, nullptr, hInstance_, this);

    dpi_ = GetDpiForWindow(hwnd_);
    RecreateFont();
    CreateControls(hwnd_);

    RECT rc{ 0, 0, Scale(kLogicalWidth, dpi_), Scale(kLogicalHeight, dpi_) };
    AdjustWindowRectExForDpi(&rc, style, FALSE, exStyle, dpi_);
    SetWindowPos(hwnd_, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    Layout();
}

SettingsWindow::~SettingsWindow() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    if (font_) {
        DeleteObject(font_);
        font_ = nullptr;
    }
}

void SettingsWindow::SetOnApply(std::function<void(const std::wstring&)> callback) {
    onApply_ = std::move(callback);
}

void SettingsWindow::RecreateFont() {
    if (font_) {
        DeleteObject(font_);
        font_ = nullptr;
    }
    int height = -MulDiv(9, static_cast<int>(dpi_), 72);
    font_ = CreateFontW(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

void SettingsWindow::CreateControls(HWND hwnd) {
    deviceLabel_ = CreateWindowExW(0, L"STATIC", L"Dispositivo de salida a monitorear:",
        WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, hwnd, nullptr, hInstance_, nullptr);

    deviceCombo_ = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | static_cast<DWORD>(CBS_DROPDOWNLIST) | WS_VSCROLL,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_DEVICE_COMBO)), hInstance_, nullptr);

    autoStartCheck_ = CreateWindowExW(0, L"BUTTON", L"Iniciar automaticamente con Windows",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | static_cast<DWORD>(BS_AUTOCHECKBOX),
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_AUTOSTART_CHECK)), hInstance_, nullptr);

    okBtn_ = CreateWindowExW(0, L"BUTTON", L"Guardar",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | static_cast<DWORD>(BS_DEFPUSHBUTTON),
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_OK)), hInstance_, nullptr);

    cancelBtn_ = CreateWindowExW(0, L"BUTTON", L"Cancelar",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CANCEL)), hInstance_, nullptr);

    for (HWND ctrl : { deviceLabel_, deviceCombo_, autoStartCheck_, okBtn_, cancelBtn_ }) {
        SendMessageW(ctrl, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    }
}

void SettingsWindow::Layout() {
    if (!hwnd_ || !deviceCombo_) return;

    RECT client{};
    GetClientRect(hwnd_, &client);
    int width = client.right - client.left;
    int height = client.bottom - client.top;

    int margin = Scale(kMargin, dpi_);
    int labelH = Scale(kLabelHeight, dpi_);
    int labelGap = Scale(kLabelGap, dpi_);
    int comboClosedH = Scale(kComboClosedHeight, dpi_);
    int comboDropH = Scale(kComboDropHeight, dpi_);
    int checkH = Scale(kCheckHeight, dpi_);
    int buttonW = Scale(kButtonWidth, dpi_);
    int buttonH = Scale(kButtonHeight, dpi_);
    int buttonGap = Scale(kButtonGap, dpi_);

    int contentWidth = std::max(width - margin * 2, Scale(kMinContentWidth, dpi_));

    int y = margin;
    MoveWindow(deviceLabel_, margin, y, contentWidth, labelH, TRUE);
    y += labelH + labelGap;

    // El "height" de un combo CBS_DROPDOWNLIST fija el limite del
    // desplegable abierto; el tamano de la caja cerrada lo da la fuente.
    MoveWindow(deviceCombo_, margin, y, contentWidth, comboDropH, TRUE);
    y += comboClosedH + Scale(kRowGap, dpi_);

    MoveWindow(autoStartCheck_, margin, y, contentWidth, checkH, TRUE);

    int buttonY = height - margin - buttonH;
    int cancelX = width - margin - buttonW;
    int okX = cancelX - buttonGap - buttonW;
    MoveWindow(okBtn_, okX, buttonY, buttonW, buttonH, TRUE);
    MoveWindow(cancelBtn_, cancelX, buttonY, buttonW, buttonH, TRUE);
}

void SettingsWindow::PopulateDeviceList() {
    SendMessageW(deviceCombo_, CB_RESETCONTENT, 0, 0);
    deviceIds_.clear();

    SendMessageW(deviceCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Predeterminado (el del sistema)"));
    deviceIds_.push_back(L"");

    for (const auto& dev : deviceManager_.EnumerateRenderDevices()) {
        SendMessageW(deviceCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(dev.name.c_str()));
        deviceIds_.push_back(dev.id);
    }

    std::wstring current = AppSettings::GetSelectedDeviceId();
    int selectIndex = 0;
    for (size_t i = 0; i < deviceIds_.size(); ++i) {
        if (deviceIds_[i] == current) {
            selectIndex = static_cast<int>(i);
            break;
        }
    }
    SendMessageW(deviceCombo_, CB_SETCURSEL, static_cast<WPARAM>(selectIndex), 0);

    SendMessageW(autoStartCheck_, BM_SETCHECK,
        AppSettings::IsAutoStartEnabled() ? BST_CHECKED : BST_UNCHECKED, 0);
}

void SettingsWindow::Show() {
    PopulateDeviceList();

    RECT winRect{};
    GetWindowRect(hwnd_, &winRect);
    int winW = winRect.right - winRect.left;
    int winH = winRect.bottom - winRect.top;

    RECT work{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    int x = (work.right - work.left - winW) / 2 + work.left;
    int y = (work.bottom - work.top - winH) / 2 + work.top;

    SetWindowPos(hwnd_, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    ShowWindow(hwnd_, SW_SHOW);
    SetForegroundWindow(hwnd_);
}

void SettingsWindow::OnOk() {
    int index = static_cast<int>(SendMessageW(deviceCombo_, CB_GETCURSEL, 0, 0));
    std::wstring deviceId = (index >= 0 && index < static_cast<int>(deviceIds_.size()))
        ? deviceIds_[index]
        : L"";

    AppSettings::SetSelectedDeviceId(deviceId);

    bool autoStart = SendMessageW(autoStartCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
    AppSettings::SetAutoStartEnabled(autoStart);

    ShowWindow(hwnd_, SW_HIDE);

    if (onApply_) onApply_(deviceId);
}

LRESULT CALLBACK SettingsWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SettingsWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<SettingsWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<SettingsWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);
    return self->HandleMessage(hwnd, msg, wParam, lParam);
}

LRESULT SettingsWindow::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_OK:
            OnOk();
            return 0;
        case IDC_CANCEL:
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        return 0;
    case WM_SIZE:
        Layout();
        return 0;
    case WM_GETMINMAXINFO: {
        auto* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        mmi->ptMinTrackSize.x = Scale(kMinLogicalWidth, dpi_);
        mmi->ptMinTrackSize.y = Scale(kMinLogicalHeight, dpi_);
        return 0;
    }
    case WM_DPICHANGED: {
        dpi_ = HIWORD(wParam);
        RecreateFont();
        for (HWND ctrl : { deviceLabel_, deviceCombo_, autoStartCheck_, okBtn_, cancelBtn_ }) {
            if (ctrl) SendMessageW(ctrl, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
        }
        auto* suggested = reinterpret_cast<RECT*>(lParam);
        SetWindowPos(hwnd, nullptr, suggested->left, suggested->top,
            suggested->right - suggested->left, suggested->bottom - suggested->top,
            SWP_NOZORDER | SWP_NOACTIVATE);
        Layout();
        return 0;
    }
    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
