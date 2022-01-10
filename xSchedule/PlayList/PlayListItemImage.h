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
class PlayerWindow;
class GIFImage;
class ScheduleOptions;

class PlayListItemImage : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _ImageFile;
	wxPoint _origin;
	wxSize _size;
    wxImage _image;
    GIFImage* _gifImage;
    PlayerWindow* _window;
    bool _topMost;
    bool _suppressVirtualMatrix;
    bool _done;
    long _duration;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemImage(wxXmlNode* node);
    PlayListItemImage(ScheduleOptions* options);
    virtual ~PlayListItemImage();
    virtual PlayListItem* Copy(const bool isClone) const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    bool GetTopMost() const { return _topMost; }
    void SetTopmost(bool topmost) { if (_topMost != topmost) { _topMost = topmost; _changeCount++; } }
    bool GetSuppressVirtualMatrix() const { return _suppressVirtualMatrix; }
    void SetSuppressVirtualMatrix(bool suppressVirtualMatrix) { if (_suppressVirtualMatrix != suppressVirtualMatrix) { _suppressVirtualMatrix = suppressVirtualMatrix; _changeCount++; } }
    virtual std::string GetNameNoTime() const override;
    void SetLocation(wxPoint pt, wxSize size) { if (_origin != pt || _size != size) { _origin = pt; _size = size; _changeCount++; } }
    void SetImageFile(const std::string& ImageFile) { if (_ImageFile != ImageFile) { _ImageFile = ImageFile; _changeCount++; } }
    std::string GetImageFile() const { return _ImageFile; }
    wxPoint GetPosition() const { return _origin; }
    wxSize GetSize() const { return _size; }
    virtual size_t GetDurationMS() const override { return _delay + _duration; }
    long GetDuration() const { return _duration; }
    void SetDuration(long duration) { if (_duration != duration) { _duration = duration; _changeCount++; } }
    virtual std::string GetTitle() const override;
    virtual std::list<std::string> GetMissingFiles() override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    virtual void Stop() override;
    virtual void Suspend(bool suspend) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};

