//(*InternalHeaders(FPPConnectDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/regex.h>
#include <wx/volume.h>
#include <wx/progdlg.h>
#include <wx/config.h>
#include <wx/dir.h>

#include "FPPConnectDialog.h"
#include "xLightsMain.h"
#include "FPP.h"
#include "xLightsXmlFile.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"

#include <log4cpp/Category.hh>
#include "../xSchedule/wxJSON/jsonreader.h"

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"
#include "../FSEQFile.h"
#include "../Parallel.h"
#include "../Discovery.h"
#include "Falcon.h"

//(*IdInit(FPPConnectDialog)
const long FPPConnectDialog::ID_SCROLLEDWINDOW1 = wxNewId();
const long FPPConnectDialog::ID_STATICTEXT1 = wxNewId();
const long FPPConnectDialog::ID_CHOICE_FILTER = wxNewId();
const long FPPConnectDialog::ID_STATICTEXT2 = wxNewId();
const long FPPConnectDialog::ID_CHOICE_FOLDER = wxNewId();
const long FPPConnectDialog::ID_STATICTEXT3 = wxNewId();
const long FPPConnectDialog::ID_PANEL2 = wxNewId();
const long FPPConnectDialog::ID_PANEL1 = wxNewId();
const long FPPConnectDialog::ID_SPLITTERWINDOW1 = wxNewId();
const long FPPConnectDialog::ID_BUTTON1 = wxNewId();
const long FPPConnectDialog::ID_BUTTON_Upload = wxNewId();
//*)

const long FPPConnectDialog::ID_MNU_SELECTALL = wxNewId();
const long FPPConnectDialog::ID_MNU_SELECTNONE = wxNewId();
const long FPPConnectDialog::ID_MNU_SELECTHIGH = wxNewId();
const long FPPConnectDialog::ID_MNU_DESELECTHIGH = wxNewId();
const long FPPConnectDialog::ID_FPP_INSTANCE_LIST = wxNewId();


static const long ID_POPUP_MNU_SORT_NAME = wxNewId();
static const long ID_POPUP_MNU_SORT_IP = wxNewId();

#define SORT_SEQ_NAME_COL 0
#define SORT_SEQ_TIME_COL 1
#define SORT_SEQ_MEDIA_COL 2

BEGIN_EVENT_TABLE(FPPConnectDialog,wxDialog)
	//(*EventTable(FPPConnectDialog)
	//*)
    EVT_TREELIST_ITEM_CONTEXT_MENU(wxID_ANY, FPPConnectDialog::SequenceListPopup)

END_EVENT_TABLE()


static const std::string CHECK_COL = "ID_UPLOAD_";
static const std::string FSEQ_COL = "ID_FSEQTYPE_";
static const std::string MEDIA_COL = "ID_MEDIA_";
static const std::string MODELS_COL = "ID_MODELS_";
static const std::string UDP_COL = "ID_UDPOUT_";
static const std::string PLAYLIST_COL = "ID_PLAYLIST_";
static const std::string UPLOAD_CONTROLLER_COL = "ID_CONTROLLER_";

FPPConnectDialog::FPPConnectDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;

	//(*Initialize(FPPConnectDialog)
	wxButton* cancelButton;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, _("FPP Upload"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_3DSASH, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxDLG_UNIT(this,wxSize(550,-1)));
	SplitterWindow1->SetMinimumPaneSize(200);
	SplitterWindow1->SetSashGravity(0.5);
	FPPInstanceList = new wxScrolledWindow(SplitterWindow1, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FPPInstanceList->SetMinSize(wxDLG_UNIT(SplitterWindow1,wxSize(-1,150)));
	FPPInstanceSizer = new wxFlexGridSizer(0, 11, 0, 0);
	FPPInstanceList->SetSizer(FPPInstanceSizer);
	FPPInstanceSizer->Fit(FPPInstanceList);
	FPPInstanceSizer->SetSizeHints(FPPInstanceList);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer3->AddGrowableCol(4);
	StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	ChoiceFilter = new wxChoice(Panel1, ID_CHOICE_FILTER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FILTER"));
	ChoiceFilter->SetSelection( ChoiceFilter->Append(_("Recursive Search")) );
	ChoiceFilter->Append(_("Only Current Directory"));
	FlexGridSizer3->Add(ChoiceFilter, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Folder:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	ChoiceFolder = new wxChoice(Panel1, ID_CHOICE_FOLDER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FOLDER"));
	FlexGridSizer3->Add(ChoiceFolder, 1, wxALL|wxEXPAND, 5);
	Selected_Label = new wxStaticText(Panel1, ID_STATICTEXT3, _("Selected: 0"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer3->Add(Selected_Label, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxEXPAND, 0);
	CheckListBoxHolder = new wxPanel(Panel1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	CheckListBoxHolder->SetMinSize(wxSize(-1,100));
	FlexGridSizer2->Add(CheckListBoxHolder, 1, wxALL|wxEXPAND, 0);
	Panel1->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel1);
	FlexGridSizer2->SetSizeHints(Panel1);
	SplitterWindow1->SplitHorizontally(FPPInstanceList, Panel1);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 5, 0, 0);
	AddFPPButton = new wxButton(this, ID_BUTTON1, _("Add FPP"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer4->Add(AddFPPButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	wxSize __SpacerSize_1 = wxDLG_UNIT(this,wxSize(50,-1));
	FlexGridSizer4->Add(__SpacerSize_1.GetWidth(),__SpacerSize_1.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Upload = new wxButton(this, ID_BUTTON_Upload, _("Upload"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Upload"));
	FlexGridSizer4->Add(Button_Upload, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_CANCEL"));
	FlexGridSizer4->Add(cancelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE_FILTER,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&FPPConnectDialog::OnChoiceFilterSelect);
	Connect(ID_CHOICE_FOLDER,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&FPPConnectDialog::OnChoiceFolderSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPConnectDialog::OnAddFPPButtonClick);
	Connect(ID_BUTTON_Upload,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPConnectDialog::OnButton_UploadClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&FPPConnectDialog::OnClose);
	//*)

    
    CheckListBox_Sequences = new wxTreeListCtrl(Panel1, wxID_ANY,
                                                wxDefaultPosition, wxDefaultSize,
                                                wxTL_CHECKBOX | wxTL_MULTIPLE, "ID_TREELISTVIEW_SEQUENCES");
    Connect(CheckListBox_Sequences->GetId(), wxEVT_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&FPPConnectDialog::OnSequenceListToggled);
    CheckListBox_Sequences->SetMinSize(wxSize(-1, 100));
    CheckListBox_Sequences->AppendColumn("Sequence", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);
    CheckListBox_Sequences->AppendColumn("Modified Date", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);
    CheckListBox_Sequences->AppendColumn("Media", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);

    wxConfigBase* config = wxConfigBase::Get();
    auto seqSortCol = config->ReadLong("xLightsFPPConnectSequenceSortCol", SORT_SEQ_NAME_COL);
    auto seqSortOrder = config->ReadBool("xLightsFPPConnectSequenceSortOrder", true);
    CheckListBox_Sequences->SetSortColumn(seqSortCol, seqSortOrder);
        
    FlexGridSizer2->Replace(CheckListBoxHolder, CheckListBox_Sequences, true);
    
    CheckListBoxHolder->Destroy();
    
    FlexGridSizer2->Layout();
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);


    wxProgressDialog prgs("Discovering FPP Instances",
                          "Discovering FPP Instances", 100, parent);
    prgs.Pulse("Discovering FPP Instances");
    prgs.Show();

    instances = FPP::GetInstances(this, outputManager);

    prgs.Pulse("Checking for mounted media drives");
    CreateDriveList();

    AddInstanceHeader("Upload", "Enable to Upload Files/Configs to this FPP Device.");
    wxPanel *p = AddInstanceHeader("Location", "Host and IP Address.");
    p->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& FPPConnectDialog::LocationPopupMenu, nullptr, this);
    
    AddInstanceHeader("Description");
    AddInstanceHeader("Mode", "FPP Mode.");
    AddInstanceHeader("Version", "FPP Software Version.");
    AddInstanceHeader("FSEQ Type", "FSEQ File Version. FPP 2.6 required for V2 formats.");
    AddInstanceHeader("Media", "Enable to Upload MP3, MP4 and WAV Media Files.");
    AddInstanceHeader("Models", "Enable to Upload Models for Display Testing.");
    AddInstanceHeader("UDP Out", "'None'- Device is not going to send Pixel data across the network. \n \n 'All' This will send pixel data over your Show Network from FPP instance to all controllers marked as 'ACTIVE'. \n \n 'Proxied' will set UDP Out only for Controllers with a Proxy IP address set.");
    AddInstanceHeader("Playlist","Select Playlist to Add Uploaded Sequences Too.");
    AddInstanceHeader("Pixel Hat/Cape", "Display Hat or Hat Attached to FPP Device, If Found.");

    prgs.Pulse("Populating FPP instance list");
    PopulateFPPInstanceList(&prgs);
    prgs.Update(100);
    prgs.Hide();
    GetFolderList(xLightsFrame::CurrentDir);

    if (config != nullptr) {
        int filterSelect = -1;
        wxString folderSelect = "";
        config->Read("FPPConnectFilterSelection", &filterSelect);
        config->Read("FPPConnectFolderSelection", &folderSelect);
        if (filterSelect != wxNOT_FOUND)
        {
            ChoiceFilter->SetSelection(filterSelect);
        }
        int ifoldSelect = ChoiceFolder->FindString(folderSelect);
        if (ifoldSelect != wxNOT_FOUND) {
            ChoiceFolder->SetSelection(ifoldSelect);
        } else {
            ChoiceFolder->SetSelection(0);
        }
    }

    LoadSequences();

    int h = SplitterWindow1->GetSize().GetHeight();
    h *= 33;
    h /= 100;
    SplitterWindow1->SetSashPosition(h);
    
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    UpdateSeqCount();
}

void FPPConnectDialog::UpdateSeqCount()
{
    uint32_t items = 0;
    uint32_t selected = 0;

    auto item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        items++;
        if (CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED)
            selected++;
        item = CheckListBox_Sequences->GetNextItem(item);
    }

    Selected_Label->SetLabel(wxString::Format("Selected: %u/%u", selected, items));
}

void FPPConnectDialog::OnSequenceListToggled(wxDataViewEvent& event)
{
    UpdateSeqCount();
}

void FPPConnectDialog::OnLocationPopupClick(wxCommandEvent &evt) {
    if (evt.GetId()== ID_POPUP_MNU_SORT_NAME) {
        SaveSettings(true);
        instances.sort(sortByName);
        PopulateFPPInstanceList();
    } else if (evt.GetId()== ID_POPUP_MNU_SORT_IP) {
        SaveSettings(true);
        instances.sort(sortByIP);
        PopulateFPPInstanceList();
    }
}

void FPPConnectDialog::LocationPopupMenu(wxContextMenuEvent& event) {
    wxMenu mnu;
    mnu.Append(ID_POPUP_MNU_SORT_NAME, "Sort by Hostname");
    mnu.Append(ID_POPUP_MNU_SORT_IP, "Sort by IP Address");
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(FPPConnectDialog::OnLocationPopupClick), NULL, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::PopulateFPPInstanceList(wxProgressDialog *prgs) {
    FPPInstanceList->Freeze();
    //remove all the children from the first upload checkbox on
    wxWindow *w = FPPInstanceList->FindWindow(CHECK_COL + "0");
    while (w) {
        wxWindow *tmp = w->GetNextSibling();
        w->Destroy();
        w = tmp;
    }
    while (FPPInstanceSizer->GetItemCount () > 11) {
        FPPInstanceSizer->Remove(11);
    }

    int row = 0;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        wxCheckBox *CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, CHECK_COL + rowStr);
        CheckBox1->SetValue(true);
        FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        std::string l = inst->hostName + " - " + inst->ipAddress;
        wxStaticText *label = new wxStaticText(FPPInstanceList, wxID_ANY, l, wxDefaultPosition, wxDefaultSize, 0, _T("ID_LOCATION_" + rowStr));
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        label = new wxStaticText(FPPInstanceList, wxID_ANY, inst->description, wxDefaultPosition, wxDefaultSize, 0, _T("ID_DESCRIPTION_" + rowStr));
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);

        auto mode = inst->mode;
        if (inst->fppType == FPP_TYPE::FPP && inst->IsVersionAtLeast(5, 0)) {
            if (inst->IsMultiSyncEnabled()) {
                mode += " w/multisync";
            }
        }
        label = new wxStaticText(FPPInstanceList, wxID_ANY, mode, wxDefaultPosition, wxDefaultSize, 0, _T("ID_MODE_" + rowStr));
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);

        label = new wxStaticText(FPPInstanceList, wxID_ANY, inst->fullVersion, wxDefaultPosition, wxDefaultSize, 0, _T("ID_VERSION_" + rowStr));
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);

        //FSEQ Type listbox
        if (inst->fppType == FPP_TYPE::FPP && inst->IsVersionAtLeast(2, 6)) {
            wxChoice *Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, FSEQ_COL + rowStr);
            wxFont font = Choice1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            Choice1->SetFont(font);
            Choice1->Append(_("V1"));
            Choice1->Append(_("V2"));
            Choice1->Append(_("V2 Sparse/zstd"));
            Choice1->Append(_("V2 Sparse/Uncompressed"));
            Choice1->SetSelection(inst->mode == "master" ? 1 : 2);
            FPPInstanceSizer->Add(Choice1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
        } else if (inst->fppType == FPP_TYPE::FALCONV4) {
            wxChoice* Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, FSEQ_COL + rowStr);
            wxFont font = Choice1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            Choice1->SetFont(font);
            Choice1->Append(_("V1"));
            Choice1->Append(_("V2 zlib"));
            Choice1->Append(_("V2 Sparse/zlib"));
            Choice1->Append(_("V2 Sparse/Uncompressed"));
            Choice1->Append(_("V2 Uncompressed"));
            Choice1->SetSelection(2);
            FPPInstanceSizer->Add(Choice1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
        } else if (inst->fppType == FPP_TYPE::ESPIXELSTICK) {
            label = new wxStaticText(FPPInstanceList, wxID_ANY, "V2 Sparse/Uncompressed", wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATIC_TEXT_FS_" + rowStr));
            FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        } else {
            label = new wxStaticText(FPPInstanceList, wxID_ANY, "V1", wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATIC_TEXT_FS_" + rowStr));
            FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }

        if (inst->fppType == FPP_TYPE::FPP) {
            if (prgs) {
                prgs->Pulse("Probing information from " + l);
            }
            CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, MEDIA_COL + rowStr);
            CheckBox1->SetValue(inst->mode != "remote");
            FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            if (inst->IsVersionAtLeast(2, 6)) {
                CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, MODELS_COL + rowStr);
                CheckBox1->SetValue(false);
                FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            } else {
                FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            }
            if (inst->IsVersionAtLeast(2, 0)) {
                wxChoice *Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, UDP_COL + rowStr);
                wxFont font = Choice1->GetFont();
                font.SetPointSize(font.GetPointSize() - 2);
                Choice1->SetFont(font);

                Choice1->Append(_("None"));
                Choice1->Append(_("All"));
                Choice1->Append(_("Proxied"));
                Choice1->SetSelection(0);
                FPPInstanceSizer->Add(Choice1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            } else {
                FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            }
            //playlist combo box
            if (inst->IsVersionAtLeast(2, 6) && !inst->IsDrive()) {
                wxComboBox *ComboBox1 = new wxComboBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxTE_PROCESS_ENTER, wxDefaultValidator, PLAYLIST_COL + rowStr);
                std::list<std::string> playlists;
                inst->LoadPlaylists(playlists);
                ComboBox1->Append(_(""));
                for (const auto& pl : playlists) {
                    ComboBox1->Append(pl);
                }
                wxFont font = ComboBox1->GetFont();
                font.SetPointSize(font.GetPointSize() - 2);
                ComboBox1->SetFont(font);
                FPPInstanceSizer->Add(ComboBox1, 1, wxALL|wxEXPAND, 0);
            } else {
                FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            }
        } else if (inst->fppType == FPP_TYPE::FALCONV4) {
            // this probably needs to be moved as this is not really a zlib thing but only the falcons end up here today so I am going to put it here for now
            CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, MEDIA_COL + rowStr);
            CheckBox1->SetValue(inst->mode != "remote");
            FPPInstanceSizer->Add(CheckBox1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);

            FPPInstanceSizer->Add(0, 0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0, 0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0, 0, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);
        }
        else {
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }

        auto c = _outputManager->GetControllers(inst->ipAddress);
        std::string m = FPP::GetModel(inst->pixelControllerType);
        if (c.size() == 1) {
            ControllerEthernet *controller = dynamic_cast<ControllerEthernet*>(c.front());
            const ControllerCaps *a = controller->GetControllerCaps();
            if (a) {
                m = a->GetModel();
            }
        }
        if (m != "") {
            std::string desc = m;
            if (inst->panelSize != "") {
                desc += " - " + inst->panelSize;
            }
            CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, desc, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, UPLOAD_CONTROLLER_COL + rowStr);
            CheckBox1->SetValue(false);
            FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        } else {
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }

        row++;
    }
    ApplySavedHostSettings();

    FPPInstanceList->FitInside();
    FPPInstanceList->SetScrollRate(10, 10);
    FPPInstanceList->ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);
    FPPInstanceList->Thaw();
}

wxPanel *FPPConnectDialog::AddInstanceHeader(const std::string &h, const std::string &tt) {
    wxPanel *Panel1 = new wxPanel(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    wxBoxSizer *BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *StaticText3 = new wxStaticText(Panel1, wxID_ANY, h, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    if(!tt.empty())
        StaticText3->SetToolTip(tt);
    BoxSizer1->Add(StaticText3, 1, wxLEFT|wxRIGHT|wxEXPAND, 5);
    Panel1->SetSizer(BoxSizer1);
    BoxSizer1->Fit(Panel1);
    BoxSizer1->SetSizeHints(Panel1);
    FPPInstanceSizer->Add(Panel1, 1, wxALL|wxEXPAND, 0);
    return Panel1;
}

void FPPConnectDialog::AddInstanceRow(const FPP &inst) {

}

void FPPConnectDialog::OnPopup(wxCommandEvent &event)
{
    int id = event.GetId();
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        bool isChecked = CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED;
        bool isSelected = CheckListBox_Sequences->IsSelected(item);
        if (id == ID_MNU_SELECTALL && !isChecked) {
            CheckListBox_Sequences->CheckItem(item);
        } else if (id == ID_MNU_SELECTNONE && isChecked) {
            CheckListBox_Sequences->UncheckItem(item);
        } else if (id == ID_MNU_SELECTHIGH && !isChecked && isSelected) {
            CheckListBox_Sequences->CheckItem(item);
        } else if (id == ID_MNU_SELECTHIGH && isChecked && isSelected) {
            CheckListBox_Sequences->UncheckItem(item);
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }
    UpdateSeqCount();
}

FPPConnectDialog::~FPPConnectDialog()
{
    unsigned int sortCol = SORT_SEQ_NAME_COL;
    bool ascendingOrder = 1;
    wxConfigBase* config = wxConfigBase::Get();
    CheckListBox_Sequences->GetSortColumn(&sortCol, &ascendingOrder);
    config->Write("xLightsFPPConnectSequenceSortCol", sortCol);
    config->Write("xLightsFPPConnectSequenceSortOrder", ascendingOrder);

	//(*Destroy(FPPConnectDialog)
	//*)

    for (const auto& a : instances) {
        delete a;
    }
}

void FPPConnectDialog::LoadSequencesFromFolder(wxString dir) const
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Scanning folder for sequences for FPP upload: %s", (const char *)dir.c_str());

    const wxString fseqDir = xLightsFrame::FseqDir;

    wxDir directory;
    directory.Open(dir);

    wxString file;
    bool fcont = directory.GetFirst(&file, "*.x*");
    static const int BUFFER_SIZE = 1024*12;
    std::vector<char> buf(BUFFER_SIZE); //12K buffer
    while (fcont) {
        if (file != "xlights_rgbeffects.xml" && file != OutputManager::GetNetworksFileName() && file != "xlights_keybindings.xml" &&
            (file.Lower().EndsWith("xml") || file.Lower().EndsWith("xsq"))) {
            // this could be a sequence file ... lets open it and check
            //just check if <xsequence" is in the first 512 bytes, parsing every XML is way too expensive
            wxFile doc(dir + wxFileName::GetPathSeparator() + file);
            SP_XmlPullParser *parser = new SP_XmlPullParser();
            size_t read = doc.Read(&buf[0], BUFFER_SIZE);
            parser->append(&buf[0], read);
            SP_XmlPullEvent * event = parser->getNext();
            int done = 0;
            int count = 0;
            bool isSequence = false;
            bool isMedia = false;
            std::string mediaName;

            while (!done) {
                if (!event) {
                    size_t read2 = doc.Read(&buf[0], BUFFER_SIZE);
                    if (read2 == 0) {
                        done = true;
                    } else {
                        parser->append(&buf[0], read2);
                    }
                } else {
                    switch (event->getEventType()) {
                        case SP_XmlPullEvent::eEndDocument:
                            done = true;
                            break;
                        case SP_XmlPullEvent::eStartTag: {
                                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                                wxString NodeName = wxString::FromAscii(stagEvent->getName());
                                count++;
                                if (NodeName == "xsequence") {
                                    isSequence = true;
                                } else if (NodeName == "mediaFile") {
                                    isMedia = true;
                                } else {
                                    isMedia = false;
                                }
                                if (count == 100) {
                                    //media file will be very early in the file, dont waste time;
                                    done = true;
                                }
                            }
                            break;
                        case SP_XmlPullEvent::eCData:
                            if (isMedia) {
                                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                                mediaName = wxString::FromAscii(stagEvent->getText()).ToStdString();
                                done = true;
                            }
                            break;
                    }
                }
                if (!done) {
                    event = parser->getNext();
                }
            }
            delete parser;

            xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());

            std::string fseqName = frame->GetFseqDirectory() + wxFileName::GetPathSeparator() + file.substr(0, file.length() - 4) + ".fseq";
            if (isSequence) {
                //need to check for existence of fseq
                if (!wxFile::Exists(fseqName)) {
                    fseqName = dir + wxFileName::GetPathSeparator() + file.substr(0, file.length() - 4) + ".fseq";
                }
                if (!wxFile::Exists(fseqName)) {
                    isSequence = false;
                }
            }
            if (mediaName != "") {
                if (!wxFile::Exists(mediaName)) {
                    wxFileName fn(mediaName);
                    for (auto &md : frame->GetMediaFolders()) {
                        std::string tmn = md + wxFileName::GetPathSeparator() + fn.GetFullName();
                        if (wxFile::Exists(tmn)) {
                            mediaName = tmn;
                            break;
                        }
                    }
                    if (!wxFile::Exists(mediaName)) {
                        const std::string fixedMN = FixFile(frame->CurrentDir, mediaName);
                        if (!wxFile::Exists(fixedMN)) {
                            logger_base.info("Could not find media: %s ", mediaName.c_str());
                            mediaName = "";
                        } else {
                            mediaName = fixedMN;
                        }
                    }
                }
            }
            logger_base.debug("XML:  %s   IsSeq:  %d    FSEQ:  %s   Media:  %s", (const char*)file.c_str(), isSequence, (const char*)fseqName.c_str(), (const char*)mediaName.c_str());
            if (isSequence) {

                // where you have show folders within show folders and sequences with the same name
                // such as when you have an imported subfolder this can create duplicates ... so lets first check
                // we dont already have the fseq file in the list

                bool found = false;
                for (auto item = CheckListBox_Sequences->GetFirstItem(); !found && item.IsOk(); item = CheckListBox_Sequences->GetNextItem(item)) {
                    if (fseqName == CheckListBox_Sequences->GetItemText(item)) {
                        found = true;
                    }
                }

                if (!found) {
                    wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(),
                        fseqName);

                    DisplayDateModified(fseqName, item);
                    if (mediaName != "") {
                        CheckListBox_Sequences->SetItemText(item, 2, mediaName);
                    }
                }
            }
        }
        fcont = directory.GetNext(&file);
    }
    if (ChoiceFilter->GetSelection() == 0)
    {
        fcont = directory.GetFirst(&file, wxEmptyString, wxDIR_DIRS);
        while (fcont)
        {
            if (file != "Backup")
            {
                LoadSequencesFromFolder(dir + wxFileName::GetPathSeparator() + file);
            }
            fcont = directory.GetNext(&file);
        }
    }
}

void FPPConnectDialog::LoadSequences()
{
    CheckListBox_Sequences->DeleteAllItems();
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    wxString freqDir = frame->GetFseqDirectory();

    if (ChoiceFolder->GetSelection() == 0) {
        LoadSequencesFromFolder(xLightsFrame::CurrentDir);
    }
    else {
        const wxString folder = ChoiceFolder->GetString(ChoiceFolder->GetSelection());
        LoadSequencesFromFolder(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder);
        freqDir = xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder;
    }

    wxDir directory;
    directory.Open(freqDir);

    wxString file;
    bool fcont = directory.GetFirst(&file, "*.?seq");
    while (fcont) {
        wxString v = freqDir + wxFileName::GetPathSeparator() + file;
        
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        bool found = false;
        while (item.IsOk()) {
            if (v == CheckListBox_Sequences->GetItemText(item)) {
                found = true;
            }
            item = CheckListBox_Sequences->GetNextItem(item);
        }
        if (!found) {
            wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(), v);
            DisplayDateModified(v, item);
            FSEQFile *file = FSEQFile::openFSEQFile(v);
            if (file != nullptr) {
                for (auto& header : file->getVariableHeaders()) {
                    if (header.code[0] == 'm' && header.code[1] == 'f') {
                        wxString mediaName = (const char*)(&header.data[0]);
                        mediaName = FixFile("", mediaName);
                        if (wxFileExists(mediaName)) {
                            CheckListBox_Sequences->SetItemText(item, 2, mediaName);
                        }
                    }
                }
            }
        }
        fcont = directory.GetNext(&file);
    }

    if (xLightsFrame::CurrentSeqXmlFile != nullptr) {
        wxString curSeq = xLightsFrame::CurrentSeqXmlFile->GetLongPath();
        if (!curSeq.StartsWith(xLightsFrame::CurrentDir)) {
            LoadSequencesFromFolder(curSeq);
        }
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        while (item.IsOk()) {
            if (curSeq == CheckListBox_Sequences->GetItemText(item)) {
                CheckListBox_Sequences->CheckItem(item);
                break;
            }
            item = CheckListBox_Sequences->GetNextItem(item);
        }
    }

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        const wxString itcsv = config->Read("FPPConnectSelectedSequences", wxEmptyString);

        if (!itcsv.IsEmpty()) {
            wxArrayString savedUploadItems = wxSplit(itcsv, ',');

            wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
            while (item.IsOk()) {
                if (savedUploadItems.Index(CheckListBox_Sequences->GetItemText(item)) != wxNOT_FOUND) {
                    CheckListBox_Sequences->CheckItem(item);
                }
                item = CheckListBox_Sequences->GetNextItem(item);
            }
        }
    }

    CheckListBox_Sequences->SetColumnWidth(2, wxCOL_WIDTH_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(1, wxCOL_WIDTH_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
}

void FPPConnectDialog::OnButton_UploadClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool cancelled = false;

    std::vector<bool> doUpload(instances.size());
    int row = 0;
    for (row = 0; row < doUpload.size(); ++row) {
        std::string rowStr = std::to_string(row);
        doUpload[row] = GetCheckValue(CHECK_COL + rowStr);
    }
    row = 0;
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());

    std::map<int, int> udpRanges;
    wxJSONValue outputs = FPP::CreateUniverseFile(_outputManager->GetControllers(), false, &udpRanges);
    wxProgressDialog prgs("", "", 1001, this, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    wxJSONValue memoryMaps = FPP::CreateModelMemoryMap(&frame->AllModels);
    std::string displayMap = FPP::CreateVirtualDisplayMap(&frame->AllModels, frame->GetDisplay2DCenter0());
    for (const auto& inst : instances) {
        inst->progressDialog = &prgs;
        inst->parent = this;
        // not in discovery so we can increate the timeouts to make sure things get transferred
        inst->defaultConnectTimeout = 5000;
        inst->messages.clear();
        std::string rowStr = std::to_string(row);
        if (!cancelled && doUpload[row]) {
            auto controller = _outputManager->GetControllers(inst->ipAddress);
            if (controller.size() == 1 && inst->ranges == "") {
                ControllerEthernet *ipcontroller = dynamic_cast<ControllerEthernet*>(controller.front());
                if (ipcontroller) {
                    uint32_t sc = ipcontroller->GetStartChannel() - 1;
                    inst->ranges = std::to_string(sc) + "-" + std::to_string(sc + ipcontroller->GetChannels() - 1);
                }
            }
            if (inst->fppType == FPP_TYPE::FPP) {
                std::string playlist = GetChoiceValue(PLAYLIST_COL + rowStr);
                if (playlist != "") {
                    cancelled |= inst->UploadPlaylist(playlist);
                }
                if (GetChoiceValueIndex(UDP_COL + rowStr) == 1) {
                    cancelled |= inst->UploadUDPOut(outputs);
                    //add the UDP ranges into the list of ranges
                    std::map<int, int> rngs(udpRanges);
                    inst->FillRanges(rngs);
                    inst->SetNewRanges(rngs);
                    inst->SetRestartFlag();
                } else if (GetChoiceValueIndex(UDP_COL + rowStr) == 2) {
                    cancelled |= inst->UploadUDPOutputsForProxy(_outputManager);
                    inst->SetRestartFlag();
                }
                if (GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr)) {
                    auto vendor = FPP::GetVendor(inst->pixelControllerType);
                    auto model = FPP::GetModel(inst->pixelControllerType);
                    //auto caps = ControllerCaps::GetControllerConfig(vendor, model, "");
                    auto c = _outputManager->GetControllers(inst->ipAddress);
                    if (c.size() == 1) {
                        cancelled |= inst->UploadPanelOutputs(&frame->AllModels, _outputManager, c.front());
                        cancelled |= inst->UploadVirtualMatrixOutputs(&frame->AllModels, _outputManager, c.front());
                        cancelled |= inst->UploadPixelOutputs(&frame->AllModels, _outputManager, c.front());
                    }
                }
                if (GetCheckValue(MODELS_COL + rowStr)) {
                    cancelled |= inst->UploadModels(memoryMaps);
                    cancelled |= inst->UploadDisplayMap(displayMap);
                    inst->SetRestartFlag();
                }
                //if restart flag is now set, restart and recheck range
                inst->Restart("", true);
            } else if (GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr) && controller.size() == 1) {
                BaseController *bc = BaseController::CreateBaseController(controller.front(), inst->ipAddress);
                bc->UploadForImmediateOutput(&frame->AllModels, _outputManager, controller.front(), this);
                delete bc;
            }
        }
        row++;
    }
    row = 0;
    for (const auto& inst : instances) {
        if (!cancelled && doUpload[row]) {
            std::string rowStr = std::to_string(row);
            // update the channel ranges now that the config has been uploaded an fppd restarted
            inst->UpdateChannelRanges();
        }
    }
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        if (CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED) {

            std::string fseq = CheckListBox_Sequences->GetItemText(item);
            std::string media = CheckListBox_Sequences->GetItemText(item, 2);

            FSEQFile *seq = FSEQFile::openFSEQFile(fseq);
            if (seq) {
                row = 0;
                int uploadCount = 0;
                for (const auto& inst : instances) {
                    std::string rowStr = std::to_string(row);
                    if (!cancelled && doUpload[row]) {
                        std::string m2 = media;
                        if (!GetCheckValue(MEDIA_COL + rowStr)) {
                            m2 = "";
                        }

                        int fseqType = 0;
                        if (inst->fppType == FPP_TYPE::FPP) {
                            fseqType = GetChoiceValueIndex(FSEQ_COL + rowStr);
                        } else if (inst->fppType == FPP_TYPE::FALCONV4) {
                            fseqType = GetChoiceValueIndex(FSEQ_COL + rowStr);
                            // need to adjust so they are unique
                            if (fseqType == 1) fseqType = 5;
                            if (fseqType == 2) fseqType = 6;
                        }
                        else {
                            fseqType = 3;
                        }
                        cancelled |= inst->PrepareUploadSequence(*seq,
                                                                fseq, m2,
                                                                fseqType);
                        
                        if (inst->WillUploadSequence()) {
                            uploadCount++;
                        }
                    }
                    row++;
                }
                if (!cancelled && uploadCount) {
                    prgs.SetTitle("Generating FSEQ Files");
                    cancelled |= !prgs.Update(0, "Generating " + wxFileName(fseq).GetFullName());
                    prgs.Show();
                    int lastDone = 0;
                    static const int FRAMES_TO_BUFFER = 50;
                    std::vector<std::vector<uint8_t>> frames(FRAMES_TO_BUFFER);
                    for (size_t x = 0; x < frames.size(); x++) {
                        frames[x].resize(seq->getMaxChannel() + 1);
                    }

                    for (size_t frame = 0; frame < seq->getNumFrames() && !cancelled; frame++) {
                        int donePct = frame * 1000 / seq->getNumFrames();
                        if (donePct != lastDone) {
                            lastDone = donePct;
                            cancelled |= !prgs.Update(donePct, "Generating " + wxFileName(fseq).GetFullName());
                            wxYield();
                        }

                        int lastBuffered = 0;
                        size_t startFrame = frame;
                        //Read a bunch of frames so each parallel thread has more info to work with before returning out here
                        while (lastBuffered < FRAMES_TO_BUFFER && frame < seq->getNumFrames()) {
                            FSEQFile::FrameData *f = seq->getFrame(frame);
                            if (f != nullptr)
                            {
                                if (!f->readFrame(&frames[lastBuffered][0], frames[lastBuffered].size()))
                                {
                                    logger_base.error("FPPConnect FSEQ file corrupt.");
                                }
                                delete f;
                            }
                            lastBuffered++;
                            frame++;
                        }
                        frame--;
                        std::function<void(FPP * &, int)> func = [startFrame, lastBuffered, &frames, &doUpload](FPP* &inst, int row) {
                            if (doUpload[row]) {
                                for (int x = 0; x < lastBuffered; x++) {
                                    inst->AddFrameToUpload(startFrame + x, &frames[x][0]);
                                }
                            }
                        };
                        parallel_for(instances, func);
                    }
                    row = 0;
                    for (const auto &inst : instances) {
                        if (!cancelled && doUpload[row]) {
                            cancelled |= inst->FinalizeUploadSequence();

                            if (inst->fppType == FPP_TYPE::FALCONV4) {
                                // a falcon
                                std::string proxy = "";
                                auto c = _outputManager->GetControllers(inst->ipAddress);
                                if (c.size() == 1) proxy = c.front()->GetFPPProxy();
                                Falcon falcon(inst->ipAddress, proxy);

                                if (falcon.IsConnected()) {
                                    std::string m2 = media;
                                    std::string rowStr = std::to_string(row);
                                    if (!GetCheckValue(MEDIA_COL + rowStr)) {
                                        if (m2 != "") {
                                            logger_base.debug("Media file %s not uploaded because media checkbox not selected.", (const char*)m2.c_str());
                                        }
                                        m2 = "";
                                    }
                                    cancelled |= !falcon.UploadSequence(inst->GetTempFile(), fseq, inst->mode == "remote" ? "" : m2, &prgs);
                                }
                                else {
                                    logger_base.debug("Upload failed as FxxV4 is not connected.");
                                    cancelled = true;
                                }
                                inst->ClearTempFile();
                            }
                        }
                        row++;
                    }
                }
            }
            delete seq;
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }
    row = 0;
    
    
    std::string messages;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        if (inst->fppType == FPP_TYPE::FPP) {
            if (!cancelled && doUpload[row]) {
                std::string playlist = GetChoiceValue(PLAYLIST_COL + rowStr);
                if (playlist != "") {
                    cancelled |= inst->UploadPlaylist(playlist);
                }
                inst->Restart("", true);
            }
        }
        if (!inst->messages.empty()) {
            messages += inst->ipAddress;
            if (inst->hostName != "" && inst->hostName != inst->ipAddress) {
                messages += "/";
                messages += inst->hostName;
            }
            messages += ":\n";
            for (auto &m : inst->messages) {
                messages += "    ";
                messages += m;
                messages += "\n";
            }
        }
        row++;
    }
    if (messages != "") {
        wxMessageBox(messages, "Problems Uploading", wxOK | wxCENTRE, this);
    }
    prgs.Update(1001);
    prgs.Hide();
    if (!cancelled) {
        SaveSettings();
        EndDialog(wxID_CLOSE);
    }
}

void FPPConnectDialog::CreateDriveList()
{
    wxArrayString drives;
#ifdef __WXMSW__
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxArrayString ud = wxFSVolume::GetVolumes(wxFS_VOL_REMOVABLE | wxFS_VOL_MOUNTED, 0);
    for (const auto &a : ud) {
        if (wxDir::Exists(a + "\\sequences")) {
            drives.push_back(a);
            logger_base.info("FPP Connect found drive %s with a \\sequences folder.", (const char*)a.c_str());
        }
    }
#elif defined(__WXOSX__)
    wxDir d;
    d.Open("/Volumes");
    wxString dir;
    bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
    while (fcont) {
        if ((dir[0] != '.')
            && (dir != "Recovery")
            && (dir != "Macintosh HD")
            && wxDir::Exists("/Volumes/" + dir + "/sequences")) { //raw USB drive mounted
            
            drives.push_back("/Volumes/" + dir + "/");
        }
        fcont = d.GetNext(&dir);
    }
#else
    bool done = false;
    wxDir d;
    d.Open("/media");
    wxString dir;
    bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
    while (fcont) {
        wxDir d2;
        d2.Open("/media/" + dir);
        wxString dir2;
        bool fcont2 = d2.GetFirst(&dir2, wxEmptyString, wxDIR_DIRS);
        while (fcont2) {
            if (dir2 == "sequences") {
                drives.push_back("/media/" + dir + "/" + dir2);
            } else if (wxDir::Exists("/media/" + dir + "/" + dir2 + "/sequences")) {
                drives.push_back("/media/" + dir + "/" + dir2);
            }
            fcont2 = d2.GetNext(&dir2);
        }
        fcont = d.GetNext(&dir);
    }
#endif

    for (const auto& a : drives) {
        FPP *inst = new FPP();
        inst->hostName = "FPP";
        inst->ipAddress = a;
        inst->minorVersion = 0;
        inst->majorVersion = 2;
        inst->fullVersion = "Unknown";
        inst->mode = "standalone";
        if (wxFile::Exists(a + "/fpp-info.json")) {
            //read version and hostname
            wxJSONValue system;
            wxJSONReader reader;
            wxString str;
            wxString drive = a;
            if (!ObtainAccessToURL(drive)) {
                wxDirDialog dlg(this, "Select FPP Directory", drive,
                                wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
                if (dlg.ShowModal() == wxID_OK) {
                    drive = dlg.GetPath();
                }
                if (!ObtainAccessToURL(drive)) {
                    continue;
                }
            }
            wxFile file(drive + "/fpp-info.json");
            if (!file.IsOpened()) {
                //could not open the file, likely not readable/writable
                continue;
            }
            
            file.ReadAll(&str);
            reader.Parse(str, &system);

            if (!system["hostname"].IsNull()) {
                inst->hostName = system["hostname"].AsString();
            }
            if (!system["type"].IsNull()) {
                inst->platform = system["type"].AsString();
            }
            if (!system["model"].IsNull()) {
                inst->model = system["model"].AsString();
            }
            if (!system["version"].IsNull()) {
                inst->fullVersion = system["version"].AsString();
            }
            if (system["minorVersion"].IsInt()) {
                inst->minorVersion = system["minorVersion"].AsInt();
            }
            if (system["majorVersion"].IsInt()) {
                inst->majorVersion = system["majorVersion"].AsInt();
            }
            if (!system["channelRanges"].IsNull()) {
                inst->ranges = system["channelRanges"].AsString();
            }
            if (!system["HostDescription"].IsNull()) {
                inst->description = system["HostDescription"].AsString();
            }
            if (!system["fppModeString"].IsNull()) {
                inst->mode = system["fppModeString"].AsString();
            }
        }
        instances.push_back(inst);
    }
}

bool FPPConnectDialog::GetCheckValue(const std::string &col) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxCheckBox *cb = dynamic_cast<wxCheckBox*>(w);
        if (cb) {
            return cb->GetValue();
        }
    }
    return false;
}
int FPPConnectDialog::GetChoiceValueIndex(const std::string &col) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxItemContainer *cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->GetSelection();
        }
    }
    return 0;
}

std::string FPPConnectDialog::GetChoiceValue(const std::string &col) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxComboBox *comboBox = dynamic_cast<wxComboBox*>(w);
        if (comboBox) {
            return comboBox->GetValue();
        }

        wxItemContainer *cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->GetStringSelection();
        }
    }
    return "";
}
void FPPConnectDialog::SetChoiceValueIndex(const std::string &col, int i) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxItemContainer *cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->SetSelection(i);
        }
    }
}
void FPPConnectDialog::SetCheckValue(const std::string &col, bool b) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxCheckBox *cb = dynamic_cast<wxCheckBox*>(w);
        if (cb) {
            return cb->SetValue(b);
        }
    }

}

void FPPConnectDialog::SaveSettings(bool onlyInsts)
{
    wxConfigBase* config = wxConfigBase::Get();
    if (!onlyInsts) {
        wxString selected = "";
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        while (item.IsOk()) {
            bool isChecked = CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED;
            if (isChecked) {
                if (selected != "") {
                    selected += ",";
                }
                selected += CheckListBox_Sequences->GetItemText(item);
            }
            item = CheckListBox_Sequences->GetNextItem(item);
        }

        config->Write("FPPConnectSelectedSequences", selected);
        config->Write("FPPConnectFilterSelection", ChoiceFilter->GetSelection());
        config->Write("FPPConnectFolderSelection", ChoiceFolder->GetString(ChoiceFolder->GetSelection()));
    }

    int row = 0;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        wxString keyPostfx = inst->ipAddress;
        keyPostfx.Replace(":", "_");
        keyPostfx.Replace("/", "_");
        keyPostfx.Replace("\\", "_");
        keyPostfx.Replace(".", "_");
        config->Write("FPPConnectUpload_" + keyPostfx, GetCheckValue(CHECK_COL + rowStr));
        config->Write("FPPConnectUploadMedia_" + keyPostfx, GetCheckValue(MEDIA_COL + rowStr));
        config->Write("FPPConnectUploadFSEQType_" + keyPostfx, GetChoiceValueIndex(FSEQ_COL + rowStr));
        config->Write("FPPConnectUploadModels_" + keyPostfx, GetCheckValue(MODELS_COL + rowStr));
        config->Write("FPPConnectUploadUDPOut_" + keyPostfx, GetChoiceValueIndex(UDP_COL + rowStr));
        config->Write("FPPConnectUploadPixelOut_" + keyPostfx, GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr));
        row++;
    }

    config->Flush();
}

void FPPConnectDialog::ApplySavedHostSettings()
{
    /*
     static const std::string CHECK_COL = "ID_UPLOAD_";
     static const std::string FSEQ_COL = "ID_FSEQTYPE_";
     static const std::string MEDIA_COL = "ID_MEDIA_";
     static const std::string MODELS_COL = "ID_MODELS_";
     static const std::string UDP_COL = "ID_UDPOUT_";
     static const std::string PLAYLIST_COL = "ID_PLAYLIST_";
     static const std::string UPLOAD_CONTROLLER_COL = "ID_CONTROLLER_";
     */


    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        int row = 0;
        for (const auto& inst : instances) {
            std::string rowStr = std::to_string(row);
            wxString keyPostfx = inst->ipAddress;
            keyPostfx.Replace(":", "_");
            keyPostfx.Replace("/", "_");
            keyPostfx.Replace("\\", "_");
            keyPostfx.Replace(".", "_");

            bool bval;
            int lval;
            if (config->Read("FPPConnectUpload_" + keyPostfx, &bval)) {
                SetCheckValue(CHECK_COL + rowStr, bval);
            }
            if (config->Read("FPPConnectUploadFSEQType_" + keyPostfx, &lval)) {
                SetChoiceValueIndex(FSEQ_COL + rowStr, lval);
            }
            if (config->Read("FPPConnectUploadMedia_" + keyPostfx, &bval)) {
                SetCheckValue(MEDIA_COL + rowStr, bval);
            }
            if (config->Read("FPPConnectUploadModels_" + keyPostfx, &bval)) {
                SetCheckValue(MODELS_COL + rowStr, bval);
            }
            if (config->Read("FPPConnectUploadUDPOut_" + keyPostfx, &lval)) {
                SetChoiceValueIndex(UDP_COL + rowStr, lval);
            }
            if (config->Read("FPPConnectUploadPixelOut_" + keyPostfx, &bval)) {
                SetCheckValue(UPLOAD_CONTROLLER_COL + rowStr, bval);
            }
            row++;
        }
    }
}


void FPPConnectDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(0);
}

void FPPConnectDialog::SequenceListPopup(wxTreeListEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MNU_SELECTALL, "Select All");
    mnu.Append(ID_MNU_SELECTNONE, "Clear Selections");
    mnu.Append(ID_MNU_SELECTHIGH, "Select Highlighted");
    mnu.Append(ID_MNU_DESELECTHIGH, "Deselect Highlighted");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&FPPConnectDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::OnAddFPPButtonClick(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "Find FPP Instance", "Enter IP address or hostname for FPP Instance");
    if (dlg.ShowModal() == wxID_OK && IsIPValidOrHostname(dlg.GetValue().ToStdString())) {
        std::string ipAd = dlg.GetValue().ToStdString();
        int curSize = instances.size();

        wxProgressDialog prgs("Gathering configuration for " + ipAd,
                              "Gathering configuration for " + ipAd, 100, this);
        prgs.Pulse("Gathering configuration for " + ipAd);
        prgs.Show();

        std::list<std::string> add;
        add.push_back(ipAd);
        
        Discovery discovery(this, _outputManager);
        FPP::PrepareDiscovery(discovery, add, false);
        discovery.Discover();
        FPP::MapToFPPInstances(discovery, instances, _outputManager);
        
        if (curSize < instances.size()) {
            int cur = 0;
            for (const auto &fpp : instances) {
                if (cur >= curSize) {
                    prgs.Pulse("Gathering configuration for " + fpp->hostName + " - " + fpp->ipAddress);
                    fpp->AuthenticateAndUpdateVersions();
                    fpp->probePixelControllerType();
                }
                cur++;
            }

            instances.sort(sortByIP);
            //it worked, we found some new instances, record this
            wxConfigBase* config = wxConfigBase::Get();
            wxString ip;
            config->Read("FPPConnectForcedIPs", &ip);
            if (!ip.Contains(dlg.GetValue())) {
                if (ip != "") {
                    ip += "|";
                }
                ip += dlg.GetValue();
                config->Write("FPPConnectForcedIPs", ip);
                config->Flush();
            }
            PopulateFPPInstanceList();
        }

        prgs.Hide();
    }
}

void FPPConnectDialog::OnChoiceFolderSelect(wxCommandEvent& event)
{
    LoadSequences();
}

void FPPConnectDialog::OnChoiceFilterSelect(wxCommandEvent& event)
{
    LoadSequences();
}

void FPPConnectDialog::GetFolderList(const wxString& folder)
{
    ChoiceFolder->Append("--Show Folder--");
    wxArrayString subfolders;
    wxDir dir(folder);
    if (!dir.IsOpened())
    {
        return;
    }
    wxString strFile;

    if (dir.GetFirst(&strFile, "*", wxDIR_HIDDEN | wxDIR_DIRS))
        subfolders.Add(strFile);

    while (dir.GetNext(&strFile))
    {
        subfolders.Add(strFile);
    }
    subfolders.Sort();
    for (const auto& subfolder: subfolders)
    {
        if(subfolder.StartsWith("Backup"))
            continue;
        if (subfolder.StartsWith("."))
            continue;
        ChoiceFolder->Append(subfolder);
    }
}

void FPPConnectDialog::DisplayDateModified(std::string const& filePath, wxTreeListItem &item) const
{ 
    if (wxFile::Exists(filePath)) {
        wxDateTime last_modified_time(wxFileModificationTime(filePath));
        CheckListBox_Sequences->SetItemText(item, 1, last_modified_time.Format(wxT("%Y-%m-%d %H:%M:%S")));
    }
}
