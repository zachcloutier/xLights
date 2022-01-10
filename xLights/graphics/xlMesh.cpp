

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "xlMesh.h"

#include <algorithm>
#include <log4cpp/Category.hh>
#include <wx/filename.h>


xlMesh::xlMesh(xlGraphicsContext *ctx, const std::string &f) : graphicsContext(ctx), filename(f) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    objectsLoaded = false;
    wxFileName fn(filename);
    
    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;
    reader_config.mtl_search_path = fn.GetPath();  // Path to material files

    if (!objects.ParseFromFile(filename, reader_config)) {
        if (!objects.Error().empty()) {
            logger_base.error("TinyObjReader: %s", objects.Error().c_str());
            return;
        }
    }
    if (!objects.Warning().empty()) {
        logger_base.warn("TinyObjReader: %s", objects.Warning().c_str());
    }
    objectsLoaded = true;
    
    
    materials.resize(objects.GetMaterials().size());
    int idx = 0;
    for (auto &m : objects.GetMaterials()) {
        float dissolve = m.dissolve * 255.0f;
        float red = m.diffuse[0] * 255.0f;
        float green = m.diffuse[1] * 255.0f;
        float blue = m.diffuse[2] * 255.0f;
        materials[idx].name = m.name;
        materials[idx].color.Set((uint8_t)red, (uint8_t)green, (uint8_t)blue, (uint8_t)dissolve);
        if (m.diffuse_texname != "") {
            wxString texName = m.diffuse_texname;
            if (!wxFileName::Exists(texName)) {
                texName = fn.GetPath() + fn.GetPathSeparator() + m.diffuse_texname;
            }
            if (!wxFileName::Exists(texName)) {
                texName = m.diffuse_texname;
                if (texName.Contains(("/"))) {
                    texName = texName.substr(texName.Last('/') + 1);
                }
                texName = fn.GetPath() + fn.GetPathSeparator() + texName;
            }
            wxImage image(texName);
            if (image.IsOk()) {
                image = image.Mirror(false);
                materials[idx].texture = ctx->createTexture(image);
                materials[idx].texture->SetName(m.diffuse_texname);
                materials[idx].texture->Finalize();
            } else {
                materials[idx].color.Set(128, 128, 128, (uint8_t)dissolve);
            }
        }
        materials[idx].origColor = materials[idx].color;
        idx++;
    }
    
    xMin = 9999999;
    yMin = 9999999;
    zMin = 9999999;
    xMax = -9999999;
    yMax = -9999999;
    zMax = -9999999;
    auto &vertices = objects.GetAttrib().vertices;
    for (int x = 0; x < vertices.size(); x += 3) {
        xMin = std::min(vertices[x], xMin);
        xMax = std::max(vertices[x], xMax);
        yMin = std::min(vertices[x + 1], yMin);
        yMax = std::max(vertices[x + 1], yMax);
        zMin = std::min(vertices[x + 2], zMin);
        zMax = std::max(vertices[x + 2], zMax);
    }
}

xlMesh::~xlMesh() {
}


void xlMesh::SetMaterialColor(const std::string materialName, const xlColor *c) {
    for (auto &m : materials) {
        if (m.name == materialName) {
            if (c) {
                m.color = *c;
                m.forceColor = true;
                materialsNeedResyncing = true;
            } else {
                m.forceColor = false;
                m.color = m.origColor;
                materialsNeedResyncing = true;
            }
        }
    }
}

