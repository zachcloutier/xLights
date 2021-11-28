/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OtherSettingsPanel.h"

//(*InternalHeaders(OtherSettingsPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"


#ifdef __WXOSX__
extern "C" {
extern bool isMetalComputeSupported();
}
#endif

//(*IdInit(OtherSettingsPanel)
const long OtherSettingsPanel::ID_STATICTEXT1 = wxNewId();
const long OtherSettingsPanel::ID_CHOICE1 = wxNewId();
const long OtherSettingsPanel::ID_CHOICE2 = wxNewId();
const long OtherSettingsPanel::ID_STATICTEXT2 = wxNewId();
const long OtherSettingsPanel::ID_TEXTCTRL1 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX7 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX2 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX3 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX4 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX5 = wxNewId();
const long OtherSettingsPanel::ID_CHECKBOX6 = wxNewId();
//*)

BEGIN_EVENT_TABLE(OtherSettingsPanel,wxPanel)
	//(*EventTable(OtherSettingsPanel)
	//*)
END_EVENT_TABLE()

OtherSettingsPanel::OtherSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    frame(f)
{
    //(*Initialize(OtherSettingsPanel)
    wxGridBagSizer* GridBagSizer1;
    wxGridBagSizer* GridBagSizer2;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxStaticText* StaticText1;

    Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
    GridBagSizer1 = new wxGridBagSizer(0, 0);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT1, _("Link save:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    GridBagSizer1->Add(StaticText2, wxGBPosition(3, 0), wxDefaultSpan, wxALL | wxEXPAND, 5);
    Choice_LinkSave = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice_LinkSave->SetSelection(Choice_LinkSave->Append(_("None")));
    Choice_LinkSave->Append(_("Controllers and Layout Tab"));
    GridBagSizer1->Add(Choice_LinkSave, wxGBPosition(3, 1), wxDefaultSpan, wxALL | wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Link controller upload:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    GridBagSizer1->Add(StaticText3, wxGBPosition(4, 0), wxDefaultSpan, wxALL | wxEXPAND, 5);
    StaticText1 = new wxStaticText(this, wxID_ANY, _("eMail Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    GridBagSizer1->Add(StaticText1, wxGBPosition(0, 0), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    eMailTextControl = new wxTextCtrl(this, ID_TEXTCTRL1, _("noone@nowhere.xlights.org"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(150, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    GridBagSizer1->Add(eMailTextControl, wxGBPosition(0, 1), wxDefaultSpan, wxALL | wxEXPAND, 5);
    HardwareVideoDecodingCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Hardware Video Decoding"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    HardwareVideoDecodingCheckBox->SetValue(false);
    GridBagSizer1->Add(HardwareVideoDecodingCheckBox, wxGBPosition(1, 0), wxDefaultSpan, wxALL | wxEXPAND, 5);
    ShaderCheckbox = new wxCheckBox(this, ID_CHECKBOX7, _("Shaders on Background Threads"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX7"));
    ShaderCheckbox->SetValue(false);
    GridBagSizer1->Add(ShaderCheckbox, wxGBPosition(1, 1), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Packaging Sequences"));
    GridBagSizer2 = new wxGridBagSizer(0, 0);
    ExcludePresetsCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Exclude Presets"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    ExcludePresetsCheckBox->SetValue(false);
    GridBagSizer2->Add(ExcludePresetsCheckBox, wxGBPosition(0, 0), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    ExcludeAudioCheckBox = new wxCheckBox(this, ID_CHECKBOX3, _("Exclude Audio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    ExcludeAudioCheckBox->SetValue(false);
    GridBagSizer2->Add(ExcludeAudioCheckBox, wxGBPosition(1, 0), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(GridBagSizer2, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
    GridBagSizer1->Add(StaticBoxSizer1, wxGBPosition(2, 0), wxDefaultSpan, wxALL | wxALIGN_LEFT, 0);
    Choice_LinkControllerUpload = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    Choice_LinkControllerUpload->SetSelection(Choice_LinkControllerUpload->Append(_("None")));
    Choice_LinkControllerUpload->Append(_("Inputs and Outputs"));
    GridBagSizer1->Add(Choice_LinkControllerUpload, wxGBPosition(4, 1), wxDefaultSpan, wxALL | wxEXPAND, 5);
    CheckBox_BatchRenderPromptIssues = new wxCheckBox(this, ID_CHECKBOX4, _("Prompt issues during batch render"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
    CheckBox_BatchRenderPromptIssues->SetValue(true);
    GridBagSizer1->Add(CheckBox_BatchRenderPromptIssues, wxGBPosition(5, 0), wxDefaultSpan, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_IgnoreVendorModelRecommendations = new wxCheckBox(this, ID_CHECKBOX5, _("Ignore vendor model recommendations"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
    CheckBox_IgnoreVendorModelRecommendations->SetValue(false);
    GridBagSizer1->Add(CheckBox_IgnoreVendorModelRecommendations, wxGBPosition(7, 0), wxDefaultSpan, wxALL | wxEXPAND, 5);
    CheckBox_PurgeDownloadCache = new wxCheckBox(this, ID_CHECKBOX6, _("Purge download cache at startup"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
    CheckBox_PurgeDownloadCache->SetValue(false);
    GridBagSizer1->Add(CheckBox_PurgeDownloadCache, wxGBPosition(6, 0), wxDefaultSpan, wxALL | wxEXPAND, 5);
    SetSizer(GridBagSizer1);
    GridBagSizer1->Fit(this);
    GridBagSizer1->SetSizeHints(this);

    Connect(ID_CHOICE1, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_CHOICE2, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_ENTER, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_CHECKBOX1, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_CHECKBOX7, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_CHECKBOX2, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_CHECKBOX3, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_CHECKBOX4, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_CHECKBOX5, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    Connect(ID_CHECKBOX6, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&OtherSettingsPanel::OnControlChanged);
    //*)

#ifdef __LINUX__
    HardwareVideoDecodingCheckBox->Hide();
    ShaderCheckbox->Hide();
#endif
#ifdef __WXOSX__
    //repurpose ShaderCheckbox for GPU rendering
    if (isMetalComputeSupported()) {
        ShaderCheckbox->SetLabel("Experimental GPU Rendering");
        ShaderCheckbox->SetToolTip("Some effects (currently just Butterfly and Blur) can be rendered on the GPU if this is enabled. This is HIGHLY experimental at this point.");
    } else {
        ShaderCheckbox->Hide();
    }
#endif
}

OtherSettingsPanel::~OtherSettingsPanel()
{
	//(*Destroy(OtherSettingsPanel)
	//*)
}

bool OtherSettingsPanel::TransferDataFromWindow() {
    frame->SetExcludeAudioFromPackagedSequences(ExcludeAudioCheckBox->IsChecked());
    frame->SetExcludePresetsFromPackagedSequences(ExcludePresetsCheckBox->IsChecked());
    frame->SetHardwareVideoAccelerated(HardwareVideoDecodingCheckBox->IsChecked());
#ifdef __WXOSX__
    frame->SetUseGPURendering(ShaderCheckbox->IsChecked());
#else
    frame->SetShadersOnBackgroundThreads(ShaderCheckbox->IsChecked());
#endif
    frame->SetUserEMAIL(eMailTextControl->GetValue());
	frame->SetLinkedSave(Choice_LinkSave->GetStringSelection());
	frame->SetLinkedControllerUpload(Choice_LinkControllerUpload->GetStringSelection());
	frame->SetPromptBatchRenderIssues(CheckBox_BatchRenderPromptIssues->GetValue());
	frame->SetIgnoreVendorModelRecommendations(CheckBox_IgnoreVendorModelRecommendations->GetValue());
	frame->SetPurgeDownloadCacheOnStart(CheckBox_PurgeDownloadCache->GetValue());
    return true;
}

bool OtherSettingsPanel::TransferDataToWindow() {
    ExcludeAudioCheckBox->SetValue(frame->ExcludeAudioFromPackagedSequences());
    ExcludePresetsCheckBox->SetValue(frame->ExcludePresetsFromPackagedSequences());
    HardwareVideoDecodingCheckBox->SetValue(frame->HardwareVideoAccelerated());
#ifdef __WXOSX__
    ShaderCheckbox->SetValue(frame->UseGPURendering());
#else
    ShaderCheckbox->SetValue(frame->ShadersOnBackgroundThreads());
#endif
    eMailTextControl->ChangeValue(frame->UserEMAIL());
	Choice_LinkSave->SetStringSelection(frame->GetLinkedSave());
	Choice_LinkControllerUpload->SetStringSelection(frame->GetLinkedControllerUpload());
	CheckBox_BatchRenderPromptIssues->SetValue(frame->GetPromptBatchRenderIssues());
	CheckBox_IgnoreVendorModelRecommendations->SetValue(frame->GetIgnoreVendorModelRecommendations());
	CheckBox_PurgeDownloadCache->SetValue(frame->GetPurgeDownloadCacheOnStart());

// Remove attempt to sneak functionality into the windows build
#ifndef __WXMSW__
#ifndef IGNORE_VENDORS
    CheckBox_IgnoreVendorModelRecommendations->SetValue(false);
    CheckBox_IgnoreVendorModelRecommendations->Hide();
#endif
#endif
	return true;
}

void OtherSettingsPanel::OnControlChanged(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
