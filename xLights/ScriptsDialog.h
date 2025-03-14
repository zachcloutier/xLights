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

//(*Headers(ScriptsDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <wx/arrstr.h>

#include <map>
#include <memory>
#include <string>

class LuaRunner;
class xLightsFrame;

class ScriptsDialog : public wxDialog
{
	xLightsFrame* _frame = nullptr;
    wxArrayString _scripts;
    std::unique_ptr<LuaRunner> _runner;

public:

	ScriptsDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~ScriptsDialog();

	//(*Declarations(ScriptsDialog)
	wxButton* Button_Clear;
	wxButton* Button_Refresh;
	wxButton* Button_Run;
	wxListBox* ListBoxScripts;
	wxStaticText* StaticText1;
	wxTextCtrl* TextCtrl_Log;
	//*)



    protected:

	//(*Identifiers(ScriptsDialog)
	static const long ID_STATICTEXT1;
	static const long ID_LISTBOX_SCRIPTS;
	static const long ID_BUTTON_RUN;
	static const long ID_BUTTON_REFRESH;
	static const long ID_BUTTON_CLEAR;
	static const long ID_TEXTCTRL_LOG;
	//*)

	static const long ID_MCU_VIEWSCRIPT;

private:

	//(*Handlers(ScriptsDialog)
	void OnButton_RefreshClick(wxCommandEvent& event);
	void OnButton_RunClick(wxCommandEvent& event);
	void OnButton_ClearClick(wxCommandEvent& event);
	//*)

	void OnListRClick(wxContextMenuEvent& event);
	void OnPopup(wxCommandEvent& event);

	void LoadScriptDir();
    void ProcessScriptDir(wxString const& dir);
    void Run_Lua_Script(wxString const& filepath)const;


	DECLARE_EVENT_TABLE()
};
