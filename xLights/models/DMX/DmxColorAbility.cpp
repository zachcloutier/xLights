/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "DmxColorAbility.h"
#include "../BaseObject.h"

DmxColorAbility::DmxColorAbility()
{
}

DmxColorAbility::~DmxColorAbility()
{
    //dtor
}

void DmxColorAbility::AddColorTypeProperties(wxPropertyGridInterface *grid) {

    wxPGProperty* p = grid->Append(new wxUIntProperty("Red Channel", "DmxRedChannel", red_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Green Channel", "DmxGreenChannel", green_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Blue Channel", "DmxBlueChannel", blue_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("White Channel", "DmxWhiteChannel", white_channel));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
}

int DmxColorAbility::OnColorPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base) {

    if ("DmxRedChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxRedChannel");
        ModelXml->AddAttribute("DmxRedChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXRedChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXRedChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXRedChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXRedChannel");
        return 0;
    } else if ("DmxGreenChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxGreenChannel");
        ModelXml->AddAttribute("DmxGreenChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXGreenChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXGreenChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXGreenChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXGreenChannel");
        return 0;
    } else if ("DmxBlueChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBlueChannel");
        ModelXml->AddAttribute("DmxBlueChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXBlueChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXBlueChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXBlueChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXBlueChannel");
        return 0;
    } else if ("DmxWhiteChannel" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxWhiteChannel");
        ModelXml->AddAttribute("DmxWhiteChannel", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DMXWhiteChannel");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxColorAbility::OnColorPropertyGridChange::DMXWhiteChannel");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxColorAbility::OnColorPropertyGridChange::DMXWhiteChannel");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxColorAbility::OnColorPropertyGridChange::DMXWhiteChannel");
        return 0;
    }
    return -1;
}
void DmxColorAbility::GetColor(xlColor &color, int transparency, int blackTransparency, bool allowSelected,
                               const xlColor *c, const std::vector<NodeBaseClassPtr> &Nodes) const {
    xlColor beam_color(xlWHITE);
    if (c != nullptr) {
        beam_color = *c;
    } else if (!allowSelected) {
        if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
            xlColor proxy = xlBLACK;
            if (white_channel > 0) {
                Nodes[white_channel - 1]->GetColor(proxy);
                beam_color = proxy;
            }

            if (proxy == xlBLACK) {
                Nodes[red_channel - 1]->GetColor(proxy);
                beam_color.red = proxy.red;
                Nodes[green_channel - 1]->GetColor(proxy);
                beam_color.green = proxy.red;
                Nodes[blue_channel - 1]->GetColor(proxy);
                beam_color.blue = proxy.red;
            }
        }
        else if (white_channel > 0) {
            xlColor proxy;
            Nodes[white_channel - 1]->GetColor(proxy);
            beam_color.red = proxy.red;
            beam_color.green = proxy.red;
            beam_color.blue = proxy.red;
        }
    }
    int trans = beam_color == xlBLACK ? blackTransparency : transparency;
    Model::ApplyTransparency(beam_color, trans, trans);
    color = beam_color;
}
