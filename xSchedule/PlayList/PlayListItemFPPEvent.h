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

class PlayListItemFPPEvent : public PlayListItem
{
protected:

    #pragma region Member Variables
    int _major = 1;
    int _minor = 1;
    std::string _ip;
    bool _started = false;
    int _method = 2;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemFPPEvent(wxXmlNode* node);
    PlayListItemFPPEvent();
    virtual ~PlayListItemFPPEvent() {};
    virtual PlayListItem* Copy(const bool isClone) const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetMajor(const int major) { if (_major != major) { _major = major; _changeCount++; } }
    void SetMinor(const int minor) { if (_minor != minor) { _minor = minor; _changeCount++; } }
    void SetIP(const std::string ip) { if (_ip != ip) { _ip = ip; _changeCount++; } }
    void SetMethod(const int method) { if (_method != method) { _method = method; _changeCount++; } }
    int GetMajor() const { return _major; }
    int GetMinor() const { return _minor; }
    std::string GetIP() const { return _ip; }
    bool GetMethod() const { return _method; }
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
