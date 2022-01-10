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

#include "BaseObject.h"
#include "ModelScreenLocation.h"
#include "ViewObjectManager.h"

class ModelPreview;
class xlGraphicsProgram;
class xlGraphicsContext;

class ViewObject : public BaseObject
{
public:
    ViewObject(const ObjectManager &manager);
    virtual ~ViewObject();

    virtual void AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    virtual void UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override {}
    virtual void AddTypeProperties(wxPropertyGridInterface *grid) override {}
    virtual void AddSizeLocationProperties(wxPropertyGridInterface* grid) override;
    virtual void AddDimensionProperties(wxPropertyGridInterface* grid) override {}
    virtual std::string GetDimension() const override
    {
        return "";
    }
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event);
    void SetFromXml(wxXmlNode* ObjectNode, bool zeroBased=false) override;
    void UpdateXmlWithScale() override;
    virtual void InitModel() = 0;

    void ReloadModelXml() override {
        GetBaseObjectScreenLocation().Reload();
        SetFromXml(ModelXml, false);
    }

    bool GetIs3dOnly() { return only_3d; }

    virtual const ModelScreenLocation &GetObjectScreenLocation() const = 0;
    virtual ModelScreenLocation &GetObjectScreenLocation() = 0;

    virtual bool Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) { return false; }
protected:

private:
    bool only_3d;
};

template <class ScreenLocation>
class ObjectWithScreenLocation : public ViewObject {
public:
    virtual const ModelScreenLocation &GetObjectScreenLocation() const {
        return screenLocation;
    }
    virtual ModelScreenLocation &GetObjectScreenLocation() {
        return screenLocation;
    }
    virtual const ModelScreenLocation &GetBaseObjectScreenLocation() const {
        return screenLocation;
    }
    virtual ModelScreenLocation &GetBaseObjectScreenLocation() {
        return screenLocation;
    }
protected:
    ObjectWithScreenLocation(const ViewObjectManager &manager) : ViewObject(manager) {}
    virtual ~ObjectWithScreenLocation() {}
    ScreenLocation screenLocation;
};
