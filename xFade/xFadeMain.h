/***************************************************************
 * Name:      xFadeMain.h
 * Purpose:   Defines Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#ifndef XFADEMAIN_H
#define XFADEMAIN_H

#ifdef _MSC_VER

#include <stdlib.h>
//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#endif

//(*Headers(xFadeFrame)
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
//*)

#include "../xLights/xLightsTimer.h"
#include <map>
#include <wx/socket.h>
#include <mutex>
#include "Settings.h"
#include "wxLED.h"
#include "UniverseData.h"

class wxDebugReportCompress;
class Emitter;
class E131Receiver;
class ArtNETReceiver;
class MIDIListener;
class wxFont;

wxDECLARE_EVENT(EVT_FLASH, wxCommandEvent);
wxDECLARE_EVENT(EVT_TAG, wxCommandEvent);

class xFadeFrame : public wxFrame
{
    const int JUKEBOXBUTTONS = 50;
    const int BUTTONWIDTH = 20;
    const int BUTTONHEIGHT = 20;

    void ValidateWindow();
    void AddFadeTimeButton(std::string label);

    std::map<int, UniverseData*> _universeData;
    std::string _leftTag;
    std::string _rightTag;
    Settings _settings;
    E131Receiver* _e131Receiver = nullptr;
    ArtNETReceiver* _artNETReceiver = nullptr;
    Emitter* _emitter = nullptr;
    std::list<MIDIListener*> _midiListeners;
    int _direction; // auto fade direction
    wxLed* Led_Left = nullptr;
    wxLed* Led_Right = nullptr;
    wxFont* _selectedButtonFont = nullptr;

    void CloseAll();
    void OpenAll();
    void InitialiseLEDs();
    void StartMIDIListeners();
    void SetFade();
    void SetTiming();
    void LoadState();
    void SaveState();
    void SetMIDIForControl(wxString controlName, float parm = 0.0);

    static const long ID_LED1;
    static const long ID_LED2;

    void PressJukeboxButton(int button, bool left);
    wxButton* GetJukeboxButton(int button, wxWindow* panel);
    int GetActiveButton(wxWindow* panel);

public:

        xFadeFrame(wxWindow* parent, wxWindowID id = -1);
        virtual ~xFadeFrame();
        void CreateDebugReport(wxDebugReportCompress *report);
        void SendReport(const wxString &loc, wxDebugReportCompress &report);

private:

        //(*Handlers(xFadeFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        void OnUITimerTrigger(wxTimerEvent& event);
        void OnButton_MiddleClick(wxCommandEvent& event);
        void OnButton_LeftClick(wxCommandEvent& event);
        void OnButton_RightClick(wxCommandEvent& event);
        void OnButton_ConnectToxLightsClick(wxCommandEvent& event);
        void OnSlider1CmdSliderUpdated(wxScrollEvent& event);
        void OnButton_ConfigureClick(wxCommandEvent& event);
        void OnSlider_LeftBrightnessCmdSliderUpdated(wxScrollEvent& event);
        void OnSlider_RightBrightnessCmdSliderUpdated(wxScrollEvent& event);
        void OnSlider_MasterBrightnessCmdSliderUpdated(wxScrollEvent& event);
        void OnTextCtrl_CrossFadeTimeText(wxCommandEvent& event);
        void OnButton_AdvanceClick(wxCommandEvent& event);
        void OnTimer_StatusTrigger(wxTimerEvent& event);
        //*)

        //(*Identifiers(xFadeFrame)
        static const long ID_STATICTEXT6;
        static const long ID_TEXTCTRL3;
        static const long ID_STATICTEXT10;
        static const long ID_TEXTCTRL4;
        static const long ID_PANEL1;
        static const long ID_BUTTON_CONNECT;
        static const long ID_BUTTON3;
        static const long ID_STATICLINE1;
        static const long ID_STATICTEXT5;
        static const long ID_SLIDER_LeftBrightness;
        static const long ID_STATICTEXT7;
        static const long ID_SLIDER_RightBrightness;
        static const long ID_STATICTEXT8;
        static const long ID_SLIDER_MasterBrightness;
        static const long ID_STATICLINE2;
        static const long ID_STATICTEXT1;
        static const long ID_TEXTCTRL_TIME;
        static const long ID_STATICTEXT2;
        static const long ID_STATICTEXT3;
        static const long ID_PANEL3;
        static const long ID_BUTTON_MIDDLE;
        static const long ID_BUTTON_LEFT;
        static const long ID_SLIDER_FADE;
        static const long ID_BUTTON_RIGHT;
        static const long ID_BUTTON_ADVANCE;
        static const long ID_STATICTEXT4;
        static const long ID_TEXTCTRL2;
        static const long ID_STATICTEXT11;
        static const long ID_TEXTCTRL5;
        static const long ID_PANEL2;
        static const long ID_STATUSBAR1;
        static const long ID_TIMER1;
        static const long ID_TIMER2;
        //*)

        //(*Declarations(xFadeFrame)
        wxButton* Button_Advance;
        wxButton* Button_Configure;
        wxButton* Button_ConnectToxLights;
        wxButton* Button_Left;
        wxButton* Button_Middle;
        wxButton* Button_Right;
        wxGridSizer* GridSizer_TimePresets;
        wxPanel* Panel_FadeTime;
        wxPanel* Panel_Left;
        wxPanel* Panel_Right;
        wxSlider* Slider1;
        wxSlider* Slider_LeftBrightness;
        wxSlider* Slider_MasterBrightness;
        wxSlider* Slider_RightBrightness;
        wxStaticLine* StaticLine1;
        wxStaticLine* StaticLine2;
        wxStaticText* StaticText10;
        wxStaticText* StaticText1;
        wxStaticText* StaticText2;
        wxStaticText* StaticText3;
        wxStaticText* StaticText4;
        wxStaticText* StaticText5;
        wxStaticText* StaticText6;
        wxStaticText* StaticText7;
        wxStaticText* StaticText8;
        wxStaticText* StaticText9;
        wxStatusBar* StatusBar1;
        wxTextCtrl* TextCtrl_CrossFadeTime;
        wxTextCtrl* TextCtrl_LeftSequence;
        wxTextCtrl* TextCtrl_LeftTag;
        wxTextCtrl* TextCtrl_RightSequence;
        wxTextCtrl* TextCtrl_RightTag;
        wxTimer Timer_Status;
        xLightsTimer UITimer;
        //*)

        DECLARE_EVENT_TABLE()

        void OnButtonClickLeft(wxCommandEvent& event);
        void OnButtonRClickLeft(wxContextMenuEvent& event);
        void OnButtonClickRight(wxCommandEvent& event);
        void OnButtonRClickRight(wxContextMenuEvent& event);
        void OnTextCtrlRClickCrossFadeTime(wxContextMenuEvent& event);
        void OnSliderRClickFade(wxContextMenuEvent& event);
        void OnSliderRClickLeftBrightness(wxContextMenuEvent& event);
        void OnSliderRClickRightBrightness(wxContextMenuEvent& event);
        void OnSliderRClickMasterBrightness(wxContextMenuEvent& event);

        void OnButtonClickFT(wxCommandEvent& event);
        void OnButtonRClickFT(wxContextMenuEvent& event);

        void OnButtonRClickFadeLeft(wxContextMenuEvent& event);
        void OnButtonRClickFadeMiddle(wxContextMenuEvent& event);
        void OnButtonRClickFadeRight(wxContextMenuEvent& event);
        void OnButtonRClickAdvance(wxContextMenuEvent& event);

        void OnMIDIEvent(wxCommandEvent& event);
        void OnFlash(wxCommandEvent& event);
        void OnTag(wxCommandEvent& event);
};

#endif // XFADEMAIN_H
