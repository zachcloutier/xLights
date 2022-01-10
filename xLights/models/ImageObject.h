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

#include "ViewObject.h"
#include "../graphics/xlGraphicsAccumulators.h"

class ModelPreview;

class ImageObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
    public:
        ImageObject(wxXmlNode *node, const ViewObjectManager &manager);
        virtual ~ImageObject();

        virtual void InitModel() override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
        virtual void UpdateTypeProperties(wxPropertyGridInterface* grid) override {}

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual bool Draw(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;

        virtual std::list<std::string> GetFileReferences() override;
        virtual bool CleanupFileLocations(xLightsFrame* frame) override;
        virtual std::list<std::string> CheckModelSettings() override;

    protected:

    private:
        std::string _imageFile;
        int width;
        int height;
        std::map<std::string, xlTexture*> _images;
        int transparency;
        float brightness;

};

