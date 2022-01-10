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

class PlayListItemFile : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _content;
    std::string _fileName;
    bool _started;
    bool _append;
    #pragma endregion 

    std::string FileReplaceTags(const std::string s);

public:

    #pragma region Constructors and Destructors
    PlayListItemFile(wxXmlNode* node);
    PlayListItemFile();
    virtual ~PlayListItemFile() {};
    virtual PlayListItem* Copy(const bool isClone) const override;
    #pragma endregion 

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetContent(const std::string& content) { if (_content != content) { _content = content; _changeCount++; } }
    void SetAppend(bool append) { if (_append != append) { _append = append; _changeCount++; } }
    void SetFileName(const std::string& fileName) { if (_fileName != fileName) { _fileName = fileName; _changeCount++; } }
    std::string GetContent() const { return _content; }
    bool GetAppend() const { return _append; }
    std::string GetFileName() const { return _fileName; }
    virtual std::string GetTitle() const override;
    #pragma endregion 

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    #pragma endregion 

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion
};
