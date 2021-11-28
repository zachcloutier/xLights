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

//(*Headers(JukeboxPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

#include <map>

class wxXmlNode;
class ButtonControl;
class MainSequencer;

class ButtonControl
{
public:

    enum class LOOKUPTYPE { LTDESCRIPTION,
                 LTMLT,
                 LTDISABLED };
    int _number;
    std::string _description;
    LOOKUPTYPE _type;
    std::string _element;
    std::string _tooltip;
    int _layer;
    int _time;

    ButtonControl(int i);
    ButtonControl(int i, std::string description, std::string tooltip);
    ButtonControl(int i, std::string element, int layer, int time, std::string tooltip);
    virtual ~ButtonControl() {}
    ButtonControl(wxXmlNode* n);
    wxXmlNode* Save();
    void SelectEffect(MainSequencer* sequencer);
    std::string GetTooltip() const { return _tooltip; }
};

class JukeboxPanel: public wxPanel
{
    const int JUKEBOXBUTTONS = 50;
    const int BUTTONWIDTH = 20;
    const int BUTTONHEIGHT = 20;

    std::map<int, ButtonControl*> _buttons;

    void SetButtonTooltip(int b, std::string tooltip);
    void ValidateWindow();

	public:

		JukeboxPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~JukeboxPanel();
        wxXmlNode* Save();
        void Load(wxXmlNode* node);
        void PlayItem(int item);
        wxString GetTooltips() const;
        wxString GetTooltipsJSON() const;
        wxString GetEffectPresentJSON() const;
        wxString GetEffectPresent() const;

		//(*Declarations(JukeboxPanel)
		wxGridSizer* GridSizer1;
		//*)

	protected:

		//(*Identifiers(JukeboxPanel)
		//*)

	private:

		//(*Handlers(JukeboxPanel)
		void OnResize(wxSizeEvent& event);
		//*)

        void OnButtonClick(wxCommandEvent& event);
        void OnButtonRClick(wxContextMenuEvent& event);

		DECLARE_EVENT_TABLE()
};
