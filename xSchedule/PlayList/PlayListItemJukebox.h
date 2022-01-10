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

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

typedef enum
{
    ENUM_STATE_BEGIN,
    ENUM_STATE_XLIGHTS,
    ENUM_STATE_NOTEBOOK,
    ENUM_STATE_SEQUENCERTAB,
    ENUM_STATE_JUKEBOX,
    ENUM_STATE_DONE
}
ENUMJUKEBOX;

class PlayListItemJukebox : public PlayListItem
{
protected:

    #pragma region Member Variables
    int _jukeboxButton;
    std::string _port;
    bool _started;
    ENUMJUKEBOX _state;
    #pragma endregion Member Variables

    int GetPort() const { if (_port == "A") return 1; else return 2; }

public:

    #pragma region Constructors and Destructors
    PlayListItemJukebox(wxXmlNode* node);
    PlayListItemJukebox();
    virtual ~PlayListItemJukebox() {};
    virtual PlayListItem* Copy(const bool isClone) const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    std::string GetNameNoTime() const override;
    void SetButton(int button) {
        if (_jukeboxButton != button) { _jukeboxButton = button; _changeCount++; }
    }
    void SetPortName(const std::string& port) {
        if (_port != port) { _port = port; _changeCount++; }
    }
    int GetButton() const { return _jukeboxButton; }
    void SetEnumState(ENUMJUKEBOX state) { _state = state; }
    ENUMJUKEBOX GetEnumState() const { return _state; }
    std::string GetPortName() const { return _port; }
    virtual std::string GetTitle() const override;
#pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    #pragma endregion Playing

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
