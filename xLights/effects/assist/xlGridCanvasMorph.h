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

#include "wx/wx.h"
#include "../../xlGridCanvas.h"
#include "../../sequencer/Effect.h"

#include <array>

wxDECLARE_EVENT(EVT_EFFECT_CHANGED, wxCommandEvent);

class xlGridCanvasMorph : public xlGridCanvas
{
    public:

        xlGridCanvasMorph(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                          const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlGridCanvasMorph();

        virtual void SetEffect(Effect* effect_);

    protected:
        void SetUndoPoint() const;

    private:

        void CalcDistanceTo(float targetX, float targetY, float* disance);
        void ProcessNearest(int targetX, int targetY, bool* done);
        bool CheckForInsideHit(int x, int y) const;
        int CheckForCornerHit(int x, int y) const;
        void mouseMoved(wxMouseEvent& event);
        void mouseRightDown(wxMouseEvent& event);
        void mouseLeftDown(wxMouseEvent& event);
        void mouseRightUp(wxMouseEvent& event);
        void mouseLeftUp(wxMouseEvent& event);
        void mouseLeftDClick(wxMouseEvent& event);
        void render(wxPaintEvent& event);
        void render();
        void DrawMorphEffect(xlGraphicsContext *ctx);
        void CreateCornerTextures(xlGraphicsContext *ctx);
        void UpdateMorphPositionsFromEffect();
        void UpdateSelectedMorphCorner(int x, int y, bool updateX = true, bool updateY = true);
        void SetMorphCorner1a(int x, int y);
        void SetMorphCorner1b(int x, int y);
        void SetMorphCorner2a(int x, int y);
        void SetMorphCorner2b(int x, int y);
        void StoreUpdatedMorphPositions();
        void SetTooltip(int x, int y);

        int x1a, x1b, x2a, x2b, y1a, y1b, y2a, y2b;
        int mSelectedCorner;
        bool mMorphStartLinked;
        bool mMorphEndLinked;
        wxBitmap corner_1a, corner_1b, corner_2a, corner_2b;
        std::array<xlTexture *, 6> mCornerTextures;
        wxPoint _startPoint;
        wxPoint _starta1;
        wxPoint _starta2;
        wxPoint _startb1;
        wxPoint _startb2;

        DECLARE_EVENT_TABLE()
};
