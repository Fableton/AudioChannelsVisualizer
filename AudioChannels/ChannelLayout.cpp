#include "ChannelLayout.h"
#include "Localization.h"

namespace ChannelLayout {
namespace {

struct BitInfo {
    DWORD bit;
    const wchar_t* name;
    const wchar_t* shortName;
    float x;
    float y;
};

// Posiciones normalizadas (0..1) en un plano visto desde arriba:
// x = izquierda(0)..derecha(1), y = frente(0)..atras(1).
constexpr BitInfo kBits[] = {
    { 0x00001, L"Front Left",            L"FL",  0.20f, 0.15f },
    { 0x00002, L"Front Right",           L"FR",  0.80f, 0.15f },
    { 0x00004, L"Center",                L"C",   0.50f, 0.05f },
    { 0x00008, L"LFE",                   L"LFE", 0.50f, 0.35f },
    { 0x00010, L"Rear Left",             L"RL",  0.20f, 0.85f },
    { 0x00020, L"Rear Right",            L"RR",  0.80f, 0.85f },
    { 0x00040, L"Front Left of Center",  L"FLC", 0.35f, 0.10f },
    { 0x00080, L"Front Right of Center", L"FRC", 0.65f, 0.10f },
    { 0x00100, L"Rear Center",           L"RC",  0.50f, 0.90f },
    { 0x00200, L"Side Left",             L"SL",  0.05f, 0.50f },
    { 0x00400, L"Side Right",            L"SR",  0.95f, 0.50f },
    { 0x00800, L"Top Center",            L"TC",  0.50f, 0.50f },
    { 0x01000, L"Top Front Left",        L"TFL", 0.25f, 0.20f },
    { 0x02000, L"Top Front Center",      L"TFC", 0.50f, 0.12f },
    { 0x04000, L"Top Front Right",       L"TFR", 0.75f, 0.20f },
    { 0x08000, L"Top Rear Left",         L"TRL", 0.25f, 0.80f },
    { 0x10000, L"Top Rear Center",       L"TRC", 0.50f, 0.88f },
    { 0x20000, L"Top Rear Right",        L"TRR", 0.75f, 0.80f },
};

std::vector<ChannelDef> FallbackByCount(UINT32 channelCount) {
    switch (channelCount) {
    case 1:
        return { { L"Center", L"C", 0.5f, 0.5f } };
    case 2:
        return {
            { L"Front Left", L"FL", 0.20f, 0.15f },
            { L"Front Right", L"FR", 0.80f, 0.15f },
        };
    case 6:
        return {
            { L"Front Left", L"FL", 0.20f, 0.15f },
            { L"Front Right", L"FR", 0.80f, 0.15f },
            { L"Center", L"C", 0.50f, 0.05f },
            { L"LFE", L"LFE", 0.50f, 0.35f },
            { L"Rear Left", L"RL", 0.20f, 0.85f },
            { L"Rear Right", L"RR", 0.80f, 0.85f },
        };
    case 8:
        return {
            { L"Front Left", L"FL", 0.20f, 0.15f },
            { L"Front Right", L"FR", 0.80f, 0.15f },
            { L"Center", L"C", 0.50f, 0.05f },
            { L"LFE", L"LFE", 0.50f, 0.35f },
            { L"Rear Left", L"RL", 0.20f, 0.85f },
            { L"Rear Right", L"RR", 0.80f, 0.85f },
            { L"Side Left", L"SL", 0.05f, 0.50f },
            { L"Side Right", L"SR", 0.95f, 0.50f },
        };
    default: {
        std::vector<ChannelDef> result;
        result.reserve(channelCount);
        for (UINT32 i = 0; i < channelCount; ++i) {
            std::wstring idx = std::to_wstring(i + 1);
            float x = channelCount > 1 ? static_cast<float>(i) / (channelCount - 1) : 0.5f;
            result.push_back({ Loc::ChannelFallbackName(static_cast<int>(i + 1)), idx, x, 0.5f });
        }
        return result;
    }
    }
}

} // namespace

std::vector<ChannelDef> Resolve(UINT32 channelCount, DWORD channelMask) {
    if (channelMask != 0) {
        std::vector<ChannelDef> result;
        for (const auto& b : kBits) {
            if (channelMask & b.bit) {
                result.push_back({ b.name, b.shortName, b.x, b.y });
            }
        }
        if (!result.empty()) {
            return result;
        }
    }
    return FallbackByCount(channelCount);
}

} // namespace ChannelLayout
