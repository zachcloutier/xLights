#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

#define GALAXY_CENTREX_MIN 0
#define GALAXY_CENTREX_MAX 100

#define GALAXY_CENTREY_MIN 0
#define GALAXY_CENTREY_MAX 100

#define GALAXY_STARTRADIUS_MIN 0
#define GALAXY_STARTRADIUS_MAX 250

#define GALAXY_ENDRADIUS_MIN 0
#define GALAXY_ENDRADIUS_MAX 250

#define GALAXY_STARTANGLE_MIN 0
#define GALAXY_STARTANGLE_MAX 360

#define GALAXY_REVOLUTIONS_MIN 0
#define GALAXY_REVOLUTIONS_MAX 3600

#define GALAXY_STARTWIDTH_MIN 0
#define GALAXY_STARTWIDTH_MAX 255

#define GALAXY_ENDWIDTH_MIN 0
#define GALAXY_ENDWIDTH_MAX 255

#define GALAXY_DURATION_MIN 0
#define GALAXY_DURATION_MAX 100

#define GALAXY_ACCEL_MIN -10
#define GALAXY_ACCEL_MAX 10

class GalaxyEffect : public RenderableEffect
{
    public:
        GalaxyEffect(int id);
        virtual ~GalaxyEffect();
    
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                         xlVertexColorAccumulator &backgrounds, xlColor* colorMask, bool ramps) override;
        virtual void SetDefaultParameters() override;
        virtual bool CanRenderPartialTimeInterval() const override { return true; }
        virtual bool SupportsRenderCache(const SettingsMap& settings) const override { return true; }

    protected:
        virtual xlEffectPanel *CreatePanel(wxWindow *parent) override;
};
