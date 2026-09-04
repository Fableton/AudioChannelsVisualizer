#include "TrayIcon.h"
#include "Localization.h"

TrayIcon::TrayIcon(HWND hwnd, UINT callbackMessage)
    : hwnd_(hwnd), callbackMessage_(callbackMessage) {
    ZeroMemory(&nid_, sizeof(nid_));
}

TrayIcon::~TrayIcon() {
    Hide();
    if (currentIcon_) {
        DestroyIcon(currentIcon_);
        currentIcon_ = nullptr;
    }
}

void TrayIcon::Show() {
    if (!currentIcon_) {
        currentIcon_ = BuildIcon({});
    }

    nid_.cbSize = sizeof(nid_);
    nid_.hWnd = hwnd_;
    nid_.uID = 1;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = callbackMessage_;
    nid_.hIcon = currentIcon_;
    wcscpy_s(nid_.szTip, L"AudioChannels");

    Shell_NotifyIconW(NIM_ADD, &nid_);

    NOTIFYICONDATAW verData = nid_;
    verData.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &verData);

    visible_ = true;
}

void TrayIcon::Hide() {
    if (visible_) {
        Shell_NotifyIconW(NIM_DELETE, &nid_);
        visible_ = false;
    }
}

void TrayIcon::Update(const std::vector<AudioEngine::ChannelSnapshot>& channels, const std::wstring& deviceName) {
    HICON newIcon = BuildIcon(channels);
    HICON oldIcon = currentIcon_;
    currentIcon_ = newIcon;

    nid_.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid_.hIcon = currentIcon_;

    std::wstring tooltip = L"AudioChannels";
    if (!deviceName.empty()) {
        tooltip += L" - " + deviceName;
        if (!channels.empty()) {
            tooltip += L" (" + Loc::ChannelCountLabel(channels.size()) + L")";
        }
    }
    wcsncpy_s(nid_.szTip, tooltip.c_str(), _TRUNCATE);

    if (visible_) {
        Shell_NotifyIconW(NIM_MODIFY, &nid_);
    }

    if (oldIcon) {
        DestroyIcon(oldIcon);
    }
}

RECT TrayIcon::GetIconRect() const {
    RECT rect{};
    NOTIFYICONIDENTIFIER nii{};
    nii.cbSize = sizeof(nii);
    nii.hWnd = hwnd_;
    nii.uID = nid_.uID;
    Shell_NotifyIconGetRect(&nii, &rect);
    return rect;
}

HICON TrayIcon::BuildIcon(const std::vector<AudioEngine::ChannelSnapshot>& channels) const {
    int size = GetSystemMetrics(SM_CXSMICON);
    if (size <= 0) size = 16;

    HDC screenDC = GetDC(nullptr);
    HDC memDC = CreateCompatibleDC(screenDC);

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = size;
    bmi.bmiHeader.biHeight = -size; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP colorBitmap = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ oldBmp = SelectObject(memDC, colorBitmap);

    if (bits) {
        ZeroMemory(bits, static_cast<size_t>(size) * size * 4);
    }

    if (!channels.empty()) {
        float radius = std::max(1.5f, size / (channels.size() <= 2 ? 6.0f : 9.0f));
        for (const auto& ch : channels) {
            int cx = static_cast<int>(ch.x * size);
            int cy = static_cast<int>(ch.y * size);
            COLORREF fill = ch.active ? RGB(0, 190, 0) : RGB(110, 110, 110);

            HBRUSH brush = CreateSolidBrush(fill);
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(25, 25, 25));
            HGDIOBJ oldBrush = SelectObject(memDC, brush);
            HGDIOBJ oldPen = SelectObject(memDC, pen);

            Ellipse(memDC,
                static_cast<int>(cx - radius), static_cast<int>(cy - radius),
                static_cast<int>(cx + radius) + 1, static_cast<int>(cy + radius) + 1);

            SelectObject(memDC, oldBrush);
            SelectObject(memDC, oldPen);
            DeleteObject(brush);
            DeleteObject(pen);
        }
    }

    // GDI no escribe el canal alfa de un DIB de 32 bpp: lo reconstruimos
    // marcando como opaco (255) todo pixel que quedo pintado (RGB != 0) y
    // transparente (0) el resto, para armar un icono con alfa real.
    if (bits) {
        BYTE* p = static_cast<BYTE*>(bits);
        int pixelCount = size * size;
        for (int i = 0; i < pixelCount; ++i) {
            BYTE b = p[i * 4 + 0];
            BYTE g = p[i * 4 + 1];
            BYTE r = p[i * 4 + 2];
            p[i * 4 + 3] = (b || g || r) ? 255 : 0;
        }
    }

    SelectObject(memDC, oldBmp);

    HBITMAP maskBitmap = CreateBitmap(size, size, 1, 1, nullptr);

    ICONINFO iconInfo{};
    iconInfo.fIcon = TRUE;
    iconInfo.hbmColor = colorBitmap;
    iconInfo.hbmMask = maskBitmap;

    HICON icon = CreateIconIndirect(&iconInfo);

    DeleteObject(colorBitmap);
    DeleteObject(maskBitmap);
    DeleteDC(memDC);
    ReleaseDC(nullptr, screenDC);

    return icon;
}
