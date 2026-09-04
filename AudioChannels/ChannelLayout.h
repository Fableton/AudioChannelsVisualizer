#pragma once
#include "framework.h"

// Traduce dwChannelMask / nChannels (formato WASAPI) a una lista ordenada
// de canales con nombre y posicion normalizada (0..1) para dibujar el
// icono de bandeja y la ventana de detalle.
namespace ChannelLayout {

struct ChannelDef {
    std::wstring name;
    std::wstring shortName;
    float x;
    float y;
};

std::vector<ChannelDef> Resolve(UINT32 channelCount, DWORD channelMask);

} // namespace ChannelLayout
