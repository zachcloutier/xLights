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

class PlayListItemCURL : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _url;
    std::string _curltype = "GET";
    std::string _body;
    std::string _contentType;
    bool _started = false;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemCURL(wxXmlNode* node);
    PlayListItemCURL();
    virtual ~PlayListItemCURL() {};
    virtual PlayListItem* Copy(const bool isClone) const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetURL(const std::string& url) { if (_url != url) { _url = url; _changeCount++; } }
    std::string GetURL() const { return _url; }
    void SetCURLType(const std::string& type) { if (_curltype != type) { _curltype = type; _changeCount++; } }
    std::string GetCURLType() const { return _curltype; }
    void SetBody(const std::string& body) { if (_body != body) { _body = body; _changeCount++; } }
    std::string GetBody() const { return _body; }
    void SetContentType(const std::string& contentType) { if (_contentType != contentType) { _contentType = contentType; _changeCount++; } }
    std::string GetContentType() const { return _contentType; }
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

