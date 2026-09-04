#include "DetailWindow.h"

namespace {
constexpr wchar_t kClassName[] = L"AudioChannelsDetailWnd";
constexpr float kFloorDb = -60.0f; // rango de la barra: -60 dB (vacia) .. 0 dB (llena)
}

DetailWindow::DetailWindow(HINSTANCE hInstance) : hInstance_(hInstance) {
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_DROPSHADOW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance_;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = kClassName;
        RegisterClassExW(&wc);
        classRegistered = true;
    }

    hwnd_ = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, kClassName, L"",
        WS_POPUP | WS_BORDER, 0, 0, kWidth, 100, nullptr, nullptr, hInstance_, this);
}

DetailWindow::~DetailWindow() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void DetailWindow::ShowNear(RECT anchor, const std::vector<AudioEngine::ChannelSnapshot>& channels, const std::wstring& deviceName) {
    channels_ = channels;
    deviceName_ = deviceName;

    int height = kHeaderHeight + kPadding + static_cast<int>(channels_.size()) * kRowHeight + kPadding;

    int x, y;
    bool hasAnchor = (anchor.right > anchor.left) && (anchor.bottom > anchor.top);
    if (hasAnchor) {
        x = anchor.right - kWidth;
        y = anchor.top - height - 8;
    } else {
        RECT work{};
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
        x = work.right - kWidth - 12;
        y = work.bottom - height - 12;
    }

    SetWindowPos(hwnd_, HWND_TOPMOST, x, y, kWidth, height, SWP_NOACTIVATE);
    ShowWindow(hwnd_, SW_SHOW);
    SetForegroundWindow(hwnd_);
    InvalidateRect(hwnd_, nullptr, TRUE);
}

void DetailWindow::Hide() {
    ShowWindow(hwnd_, SW_HIDE);
}

void DetailWindow::Refresh(const std::vector<AudioEngine::ChannelSnapshot>& channels) {
    channels_ = channels;
    InvalidateRect(hwnd_, nullptr, FALSE);
}

bool DetailWindow::IsVisible() const {
    return hwnd_ != nullptr && IsWindowVisible(hwnd_) != FALSE;
}

LRESULT CALLBACK DetailWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    DetailWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<DetailWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<DetailWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (self) self->OnPaint(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) {
            ShowWindow(hwnd, SW_HIDE);
        }
        return 0;
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

void DetailWindow::OnPaint(HDC hdc) {
    RECT rc;
    GetClientRect(hwnd_, &rc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

    HBRUSH bg = CreateSolidBrush(RGB(245, 245, 245));
    FillRect(memDC, &rc, bg);
    DeleteObject(bg);

    SetBkMode(memDC, TRANSPARENT);

    HFONT headerFont = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT rowFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    HGDIOBJ oldFont = SelectObject(memDC, headerFont);
    RECT headerRect{ kPadding, 8, rc.right - kPadding, kHeaderHeight };
    std::wstring header = deviceName_.empty() ? L"AudioChannels" : deviceName_;
    SetTextColor(memDC, RGB(30, 30, 30));
    DrawTextW(memDC, header.c_str(), -1, &headerRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
    SelectObject(memDC, rowFont);

    int y = kHeaderHeight;
    for (const auto& ch : channels_) {
        RECT rowRect{ kPadding, y, rc.right - kPadding, y + kRowHeight };

        HBRUSH dotBrush = CreateSolidBrush(ch.active ? RGB(0, 170, 0) : RGB(160, 160, 160));
        HPEN dotPen = CreatePen(PS_SOLID, 1, RGB(90, 90, 90));
        HGDIOBJ oldBrush = SelectObject(memDC, dotBrush);
        HGDIOBJ oldPen = SelectObject(memDC, dotPen);
        Ellipse(memDC, rowRect.left, y + 10, rowRect.left + 10, y + 20);
        SelectObject(memDC, oldBrush);
        SelectObject(memDC, oldPen);
        DeleteObject(dotBrush);
        DeleteObject(dotPen);

        RECT nameRect{ rowRect.left + 18, y, rowRect.left + 140, y + 18 };
        SetTextColor(memDC, RGB(30, 30, 30));
        DrawTextW(memDC, ch.name.c_str(), -1, &nameRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);

        RECT barOuter{ rowRect.left + 18, y + 20, rowRect.right - 68, y + 28 };
        HBRUSH barBg = CreateSolidBrush(RGB(222, 222, 222));
        FillRect(memDC, &barOuter, barBg);
        DeleteObject(barBg);

        float t = ch.active ? std::clamp((ch.levelDb - kFloorDb) / (0.0f - kFloorDb), 0.0f, 1.0f) : 0.0f;
        RECT barFill = barOuter;
        barFill.right = barOuter.left + static_cast<LONG>((barOuter.right - barOuter.left) * t);
        if (barFill.right > barFill.left) {
            HBRUSH barFg = CreateSolidBrush(RGB(0, 170, 0));
            FillRect(memDC, &barFill, barFg);
            DeleteObject(barFg);
        }

        RECT dbRect{ rowRect.right - 64, y, rowRect.right, y + 18 };
        std::wstring dbText = ch.active
            ? (std::to_wstring(static_cast<int>(std::lround(ch.levelDb))) + L" dB")
            : L"--";
        SetTextColor(memDC, RGB(100, 100, 100));
        DrawTextW(memDC, dbText.c_str(), -1, &dbRect, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);

        y += kRowHeight;
    }

    SelectObject(memDC, oldFont);
    DeleteObject(headerFont);
    DeleteObject(rowFont);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}
