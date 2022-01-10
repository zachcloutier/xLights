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

#include <map>
#include "../../graphics/xlGraphicsAccumulators.h"
#include <glm/glm.hpp>

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;

class ModelPreview;
class wxXmlNode;

class DmxImage
{
    public:
        DmxImage(wxXmlNode* node, wxString _name);
        virtual ~DmxImage();

        void Init(BaseObject* base);
        bool GetExists() { return obj_exists; }

        void AddTypeProperties(wxPropertyGridInterface* grid);
        void UpdateTypeProperties(wxPropertyGridInterface* grid) {}

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        void Draw(BaseObject* base, ModelPreview* preview, xlGraphicsProgram *pg,
                  glm::mat4 &motion_matrix,
                  int transparency, float brightness, bool only_image,
                  float pivot_offset_x, float pivot_offset_y, bool rotation, bool use_pivot);

        void SetOffsetZ(float value, BaseObject* base);
        bool ImageSelected() const { return image_selected; }
        void ClearImageSelected() { image_selected = false; }
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
        void SetScaleX(float value, BaseObject* base);
        void SetScaleY(float value, BaseObject* base);

        void Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const;
        void Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const;

    protected:

    private:
        wxXmlNode* node_xml;
        std::string _imageFile;
        int width;
        int height;
        std::map<std::string, xlTexture*> _images;
        bool obj_exists;
        bool image_selected;

        float offset_x;
        float offset_y;
        float offset_z;
        float scalex;
        float scaley;
        float scalez;
        float rotatex;
        float rotatey;
        float rotatez;
        wxString base_name;

};

