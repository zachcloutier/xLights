#include "OffEffect.h"
#include "OffPanel.h"

#include "../RenderBuffer.h"

#include "../../include/Off.xpm"

OffEffect::OffEffect(int i) : RenderableEffect(i, "Off", Off, Off, Off, Off, Off)
{
    //ctor
}

OffEffect::~OffEffect()
{
    //dtor
}

wxPanel *OffEffect::CreatePanel(wxWindow *parent) {
    return new OffPanel(parent);
}

void OffEffect::Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) {
    const size_t maxh = buffer.BufferHt;
    const size_t maxw = buffer.BufferWi;
    //  Every Node, every frame set to BLACK
    for (size_t x = 0; x < maxw; x++) {
        for (size_t y = 0; y < maxh; y++) {
            size_t idx = y * maxw + x;
            buffer.pixels[idx].rgba = 0xFF000000;
        }
    }
}
