#include "PlasmaEffect.h"
#include "PlasmaPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/plasma-16.xpm"
#include "../../include/plasma-24.xpm"
#include "../../include/plasma-32.xpm"
#include "../../include/plasma-48.xpm"
#include "../../include/plasma-64.xpm"

PlasmaEffect::PlasmaEffect(int id) : RenderableEffect(id, "Plasma", plasma_16, plasma_24, plasma_32, plasma_48, plasma_64)
{
    //ctor
}

PlasmaEffect::~PlasmaEffect()
{
    //dtor
}
wxPanel *PlasmaEffect::CreatePanel(wxWindow *parent) {
    return new PlasmaPanel(parent);
}

#define PLASMA_NORMAL_COLORS    0
#define PLASMA_PRESET1          1
#define PLASMA_PRESET2          2
#define PLASMA_PRESET3          3
#define PLASMA_PRESET4          4

static inline int GetPlasmaColorScheme(const std::string &ColorSchemeStr) {
    if (ColorSchemeStr == "Preset Colors 1") {
        return PLASMA_PRESET1;
    } else if (ColorSchemeStr == "Preset Colors 2") {
        return PLASMA_PRESET2;
    } else if (ColorSchemeStr == "Preset Colors 3") {
        return PLASMA_PRESET3;
    } else if (ColorSchemeStr == "Preset Colors 4") {
        return PLASMA_PRESET4;
    }
    return PLASMA_NORMAL_COLORS;
}

void PlasmaEffect::SetDefaultParameters(Model *cls) {
    PlasmaPanel *pp = (PlasmaPanel*)panel;
    if (pp == nullptr) {
        return;
    }

    pp->BitmapButton_Plasma_SpeedVC->SetActive(false);

    SetSliderValue(pp->Slider_Plasma_Style, 1);
    SetSliderValue(pp->Slider_Plasma_Line_Density, 1);
    SetSliderValue(pp->Slider_Plasma_Speed, 10);

    SetChoiceValue(pp->Choice_Plasma_Color, "Normal");
}


static void DoPresets1(RenderBuffer &buffer, float time, int Style,
                          int Line_Density) {

    const size_t maxx = buffer.BufferWi;
    const size_t maxy = buffer.BufferHt;
    float sin_time_5 = buffer.sin(time / 5);
    float cos_time_3 = buffer.cos(time / 3);
    float sin_time_2 = buffer.sin(time / 2);

    for (size_t x = 0; x < maxx; ++x) {
        float rx = ((float)x / (maxx - 1)); // rx is now in the range 0.0 to 1.0
        float rx2 = rx * rx;
        float cx = rx + .5*sin_time_5;
        float cx2 = cx*cx;
        float sin_rx_time = buffer.sin(rx + time);
        
        // 1st equation
        float v1 = buffer.sin(rx * 10 + time);
        #pragma clang loop vectorize(enable)
        for (size_t y = 0; y < maxy; ++y) {
            // reference: http://www.bidouille.org/prog/plasma
            
            float ry = y;
            ry /= ((float)(maxy-1)) ;
            
            float v = v1;
            //  second equation
            v+=buffer.sin(10*(rx*sin_time_2+ry*cos_time_3)+time);

            //  third equation
            float cy=ry+.5*cos_time_3;
            v+=buffer.sin(std::sqrt((Style*50)*((cx2)+(cy*cy))+time));
            
            v += sin_rx_time;
            v += buffer.sin((ry+time)/2.0);
            v += buffer.sin((rx+ry+time)/2.0);
            v += buffer.sin(std::sqrt(rx2+ry*ry)+time);
            v = v/2.0;
            
            float vldpi = v*Line_Density*M_PI;
            
            size_t idx = y*maxx+x;
            float sinvldpi = (buffer.sin(vldpi) + 1) * 128;
            buffer.pixels[idx].SetViaRGBA(sinvldpi, sinvldpi, 0);
        }
    }
}
static void DoPresets2(RenderBuffer &buffer, float time, int Style,
                          int Line_Density) {
    
    const size_t maxx = buffer.BufferWi;
    const size_t maxy = buffer.BufferHt;
    float sin_time_5 = buffer.sin(time / 5);
    float cos_time_3 = buffer.cos(time / 3);
    float sin_time_2 = buffer.sin(time / 2);
    
    for (size_t x = 0; x < maxx; ++x) {
        float rx = ((float)x / (maxx - 1)); // rx is now in the range 0.0 to 1.0
        float rx2 = rx * rx;
        float cx = rx + .5*sin_time_5;
        float cx2 = cx*cx;
        float sin_rx_time = buffer.sin(rx + time);
        
        // 1st equation
        float v1 = buffer.sin(rx * 10 + time);
#pragma clang loop vectorize(enable)
        for (size_t y = 0; y < maxy; ++y) {
            // reference: http://www.bidouille.org/prog/plasma
            
            float ry = y;
            ry /= ((float)(maxy-1));
            
            float v = v1;
            //  second equation
            v+=buffer.sin(10*(rx*sin_time_2+ry*cos_time_3)+time);
            
            //  third equation
            float cy=ry+.5*cos_time_3;
            v+=buffer.sin(std::sqrt((Style*50)*((cx2)+(cy*cy))+time));
            
            v += sin_rx_time;
            v += buffer.sin((ry+time)/2.0);
            v += buffer.sin((rx+ry+time)/2.0);
            v += buffer.sin(std::sqrt(rx2+ry*ry)+time);
            v = v/2.0;
            
            float vldpi = v*Line_Density*M_PI;
            
            size_t idx = y*maxx+x;
            //PLASMA_PRESET2
            float sinvldpi = (buffer.sin(vldpi) + 1) * 128;
            float cosvldpi = (buffer.cos(vldpi) + 1) * 128;
            buffer.pixels[idx].SetViaRGBA(1, cosvldpi, sinvldpi);
        }
    }
}
static void DoPresets3(RenderBuffer &buffer, float time, int Style,
                          int Line_Density) {
    
    const size_t maxx = buffer.BufferWi;
    const size_t maxy = buffer.BufferHt;
    float sin_time_5 = buffer.sin(time / 5);
    float cos_time_3 = buffer.cos(time / 3);
    float sin_time_2 = buffer.sin(time / 2);
    
    for (size_t x = 0; x < maxx; ++x) {
        float rx = ((float)x / (maxx - 1)); // rx is now in the range 0.0 to 1.0
        float rx2 = rx * rx;
        float cx = rx + .5*sin_time_5;
        float cx2 = cx*cx;
        float sin_rx_time = buffer.sin(rx + time);
        
        // 1st equation
        float v1 = buffer.sin(rx * 10 + time);
        #pragma clang loop vectorize(enable)
        for (size_t y = 0; y < maxy; ++y) {
            // reference: http://www.bidouille.org/prog/plasma
            
            float ry = y;
            ry /= ((float)(maxy-1)) ;
            
            float v = v1;
            //  second equation
            v+=buffer.sin(10*(rx*sin_time_2+ry*cos_time_3)+time);
            
            //  third equation
            float cy=ry+.5*cos_time_3;
            v+=buffer.sin(std::sqrt((Style*50)*((cx2)+(cy*cy))+time));
            
            v += sin_rx_time;
            v += buffer.sin((ry+time)/2.0);
            v += buffer.sin((rx+ry+time)/2.0);
            v += buffer.sin(std::sqrt(rx2+ry*ry)+time);
            v = v/2.0;
            
            float vldpi = v*Line_Density*M_PI;
            
            size_t idx = y*maxx+x;
            float red = (buffer.sin(vldpi) + 1) * 128;
            float green = (buffer.sin(vldpi + 2 * M_PI / 3.0) + 1) * 128;
            float blue = (buffer.sin(vldpi + 4 * M_PI / 3.0) + 1) * 128;
            buffer.pixels[idx].SetViaRGBA(red, green, blue);
        }
    }
}
static void DoPresets4(RenderBuffer &buffer, float time, int Style,
                          int Line_Density) {
    
    const size_t maxx = buffer.BufferWi;
    const size_t maxy = buffer.BufferHt;
    float sin_time_5 = buffer.sin(time / 5);
    float cos_time_3 = buffer.cos(time / 3);
    float sin_time_2 = buffer.sin(time / 2);
    
    for (size_t x = 0; x < maxx; ++x) {
        float rx = ((float)x / (maxx - 1)); // rx is now in the range 0.0 to 1.0
        float rx2 = rx * rx;
        float cx = rx + .5*sin_time_5;
        float cx2 = cx*cx;
        float sin_rx_time = buffer.sin(rx + time);
        
        // 1st equation
        float v1 = buffer.sin(rx * 10 + time);
        #pragma clang loop vectorize(enable)
        for (size_t y = 0; y < maxy; ++y) {
            // reference: http://www.bidouille.org/prog/plasma
            
            float ry = y;
            ry /= ((float)(maxy-1)) ;
            
            float v = v1;
            //  second equation
            v+=buffer.sin(10*(rx*sin_time_2+ry*cos_time_3)+time);
            
            //  third equation
            float cy=ry+.5*cos_time_3;
            v+=buffer.sin(std::sqrt((Style*50)*((cx2)+(cy*cy))+time));
            
            v += sin_rx_time;
            v += buffer.sin((ry+time)/2.0);
            v += buffer.sin((rx+ry+time)/2.0);
            v += buffer.sin(std::sqrt(rx2+ry*ry)+time);
            v = v/2.0;
            
            float vldpi = v*Line_Density*M_PI;
            
            size_t idx = y*maxx+x;
            //PLASMA_PRESET4
            float c = (buffer.sin(vldpi) + 1) * 128;
            buffer.pixels[idx].SetViaRGBA(c, c, c);
        }
    }
}

static void DoNormalColors(RenderBuffer &buffer, float time, int Style,
                          int Line_Density) {
    
    const size_t maxx = buffer.BufferWi;
    const size_t maxy = buffer.BufferHt;
    float sin_time_5 = buffer.sin(time / 5);
    float cos_time_3 = buffer.cos(time / 3);
    float sin_time_2 = buffer.sin(time / 2);
    xlColor c;
    for (size_t x = 0; x < maxx; ++x) {
        float rx = ((float)x / (maxx - 1)); // rx is now in the range 0.0 to 1.0
        float rx2 = rx * rx;
        float cx = rx + .5*sin_time_5;
        float cx2 = cx*cx;
        float sin_rx_time = buffer.sin(rx + time);
        
        // 1st equation
        float v1 = buffer.sin(rx * 10 + time);
#pragma clang loop vectorize(enable)
        for (size_t y = 0; y < maxy; ++y) {
            // reference: http://www.bidouille.org/prog/plasma
            
            float ry = y;
            ry /= ((float)(maxy-1)) ;
            
            float v = v1;
            //  second equation
            v+=buffer.sin(10*(rx*sin_time_2+ry*cos_time_3)+time);
            
            //  third equation
            float cy=ry+.5*cos_time_3;
            v+=buffer.sin(std::sqrt((Style*50)*((cx2)+(cy*cy))+time));
            
            v += sin_rx_time;
            v += buffer.sin((ry+time)/2.0);
            v += buffer.sin((rx+ry+time)/2.0);
            v += buffer.sin(std::sqrt(rx2+ry*ry)+time);
            v = v/2.0;
            
            float vldpi = v*Line_Density*M_PI;
            
            size_t idx = y*maxx+x;
            double h = (buffer.sin (vldpi + 2 * M_PI/3.0) + 1) * 0.5;
            buffer.GetMultiColorBlend(h,false,c);
            buffer.pixels[idx] = c;
        }
    }
}

void PlasmaEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    int Style = SettingsMap.GetInt("SLIDER_Plasma_Style", 1);
    int Line_Density = SettingsMap.GetInt("SLIDER_Plasma_Line_Density", 1);
    int PlasmaSpeed = GetValueCurveInt("Plasma_Speed", 10, SettingsMap, oset);
    std::string PlasmaDirectionStr = SettingsMap["CHOICE_Plasma_Direction"];

    int PlasmaDirection = 0; //fixme?
    int ColorScheme = GetPlasmaColorScheme(SettingsMap["CHOICE_Plasma_Color"]);

    int curState = (buffer.curPeriod - buffer.curEffStartPer) * PlasmaSpeed * buffer.frameTimeInMs / 50;
    float offset=float(curState)/200.0;

    int state = (buffer.curPeriod - buffer.curEffStartPer); // frames 0 to N
    float Speed_plasma = (101-PlasmaSpeed)*3; // we want a large number to divide by
    float time = (state+1.0)/Speed_plasma;

    if (PlasmaDirection==1) offset = -offset;

    switch (ColorScheme) {
        case PLASMA_PRESET1:
            DoPresets1(buffer, time, Style, Line_Density);
            break;
        case PLASMA_PRESET2:
            DoPresets2(buffer, time, Style, Line_Density);
            break;
        case PLASMA_PRESET3:
            DoPresets3(buffer, time, Style, Line_Density);
            break;
        case PLASMA_PRESET4:
            DoPresets4(buffer, time, Style, Line_Density);
            break;
        default:
            DoNormalColors(buffer, time, Style, Line_Density);
            break;
    }
}
