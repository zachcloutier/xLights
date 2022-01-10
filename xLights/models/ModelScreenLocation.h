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

#define CUR_MODEL_POS_VER      "5"

#define NO_HANDLE              -1
#define CENTER_HANDLE          0
#define L_TOP_HANDLE           1
#define R_TOP_HANDLE           2
#define R_BOT_HANDLE           3
#define L_BOT_HANDLE           4
#define ROTATE_HANDLE          5
#define L_TOP_HANDLE_Z         6
#define R_TOP_HANDLE_Z         7
#define R_BOT_HANDLE_Z         8
#define L_BOT_HANDLE_Z         9
#define START_HANDLE           1
#define END_HANDLE             2
#define SHEAR_HANDLE           3

// Lower 20 bits reserved to store handle positions and these
// constants are modifiers to indicate special handles
#define HANDLE_MASK    0x00FFFFF
#define HANDLE_SEGMENT 0x1000000
#define HANDLE_AXIS    0x0200000
#define HANDLE_CP0     0x0400000
#define HANDLE_CP1     0x0800000

class wxXmlNode;
class ModelPreview;
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class wxCursor;
class PreviewCamera;

#include <shared_mutex>
#include <vector>
#include "Node.h"
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class BezierCurveCubic3D;
class xlGraphicsProgram;
class xlGraphicsContext;
class xlVertexColorAccumulator;

class ModelScreenLocation
{
protected:
    float GetAxisArrowLength(float zoom, int scale) const;
    float GetAxisHeadLength(float zoom, int scale) const;
    float GetAxisRadius(float zoom, int scale) const;
    float GetRectHandleWidth(float zoom, int scale) const;
    public:

    enum class MSLAXIS {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,
        NO_AXIS
    };

    enum class MSLTOOL {
        TOOL_TRANSLATE,
        TOOL_SCALE,
        TOOL_ROTATE,
        TOOL_XY_TRANS,
        TOOL_ELEVATE,
        TOOL_NONE
    };

    enum class MSLUPGRADE {
        MSLUPGRADE_NOT_NEEDED,
        MSLUPGRADE_SKIPPED,
        MSLUPGRADE_EXEC_DONE,
        MSLUPGRADE_EXEC_READ
    };

    MSLAXIS NextAxis(MSLAXIS axis);
    virtual void Read(wxXmlNode* node) = 0;
    virtual void Write(wxXmlNode *node) = 0;
    virtual MSLUPGRADE CheckUpgrade(wxXmlNode *node) = 0;
    void Reload() { rotation_init = true; }

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const = 0;
    virtual void TranslatePoint(float &x, float &y, float &z) const = 0;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const = 0;

    virtual std::string GetDimension(float factor = 1.0) const = 0;
    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const = 0;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const = 0;
    virtual bool HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const = 0;
    virtual wxCursor CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle, float zoom, int scale) const;

    //new drawing code
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale) const { return false; };
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding) const { return false; };
    void DrawAxisTool(glm::vec3& pos, xlGraphicsProgram *program, float zoom, int scale) const;

    
    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) = 0;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) = 0;
    virtual void MouseDown(bool value) { mouse_down = value; }

    virtual bool Rotate(MSLAXIS axis, float factor) = 0;
    virtual bool Scale(const glm::vec3& factor) = 0;

    virtual void SelectHandle(int handle) = 0;
    virtual int GetSelectedHandle() const = 0;
    virtual int GetNumHandles() const = 0;
    virtual void SelectSegment(int segment) = 0;
    virtual int GetSelectedSegment() const = 0;
    virtual bool HasCurve(int segment) const = 0;
    virtual void SetCurve(int segment, bool create = true) = 0;
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) = 0;
    virtual void InsertHandle(int after_handle, float zoom, int scale) = 0;
    virtual void DeleteHandle(int handle) = 0;
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) = 0;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) = 0;
    virtual void UpdateBoundingBox(float width, float height, float depth);

    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const = 0;
    virtual void AddDimensionProperties(wxPropertyGridInterface* propertyEditor, float factor = 1.0) const = 0;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) = 0;
    virtual bool IsCenterBased() const = 0;
    virtual float GetVScaleFactor() const {return 1.0;}

    virtual void SetPosition(float posx, float posy) = 0;
    virtual void AddOffset(float deltax, float deltay, float deltaz);

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) const = 0;
    virtual glm::vec2 GetScreenPosition(int screenwidth, int screenheight, ModelPreview* preview, PreviewCamera* camera, float &sx, float &sy, float &sz) const;
    virtual float GetHcenterPos() const = 0;
    virtual float GetVcenterPos() const = 0;
    virtual float GetDcenterPos() const = 0;
    virtual void SetHcenterPos(float f) = 0;
    virtual void SetVcenterPos(float f) = 0;
    virtual void SetDcenterPos(float f) = 0;

    //in screen coordinates
    virtual float GetTop() const = 0;
    virtual float GetLeft() const = 0;
    virtual float GetRight() const = 0;
    virtual float GetBottom() const = 0;
    virtual float GetFront() const = 0;
    virtual float GetBack() const = 0;
    virtual void SetTop(float i) = 0;
    virtual void SetLeft(float i) = 0;
    virtual void SetRight(float i) = 0;
    virtual void SetBottom(float i) = 0;
    virtual void SetFront(float i) = 0;
    virtual void SetBack(float i) = 0;
    virtual void SetMWidth(float w) = 0;
    virtual void SetMHeight(float h) = 0;
    virtual void SetMDepth(float d) = 0;
    virtual float GetMWidth() const = 0;
    virtual float GetMHeight() const = 0;
    virtual float GetMDepth() const = 0;
    virtual float GetRestorableMWidth() const { return GetMWidth(); }
    virtual float GetRestorableMHeight() const { return GetMHeight(); }
    virtual float GetRestorableMDepth() const { return GetMDepth(); }
    virtual void RotateAboutPoint(glm::vec3 position, glm::vec3 angle);
    virtual void SetEdit(bool val) { }
    virtual bool GetEdit() const { return false; }
    virtual void SetToolSize(int sz) { tool_size = sz; };
    virtual void* GetRawData() { return nullptr; }

    void SetRenderSize(float NewWi, float NewHt, float NewDp = 0.0f);
    void AdjustRenderSize(float NewWi, float NewHt, float NewDp, wxXmlNode* node);
    bool IsLocked() const { return _locked; }
    void Lock(bool value = true) { _locked = value; }
    float GetRenderHt() const { return RenderHt; }
    float GetRenderWi() const { return RenderWi; }
    float GetRenderDp() const { return RenderDp; }
    float RenderHt = 0.0f;
    float RenderWi = 0.0f;
    float RenderDp = 0.0f; // size of the rendered output
    int previewW = -1;
    int previewH = -1;

    struct xlPoint {
        float x;
        float y;
        float z;
    };

    void AddASAPWork(uint32_t work, const std::string& from);
    void SetDefaultMatrices() const;  // for models that draw themselves
    virtual void SetActiveHandle(int handle);
    int GetActiveHandle() const { return active_handle; }
    virtual void SetActiveAxis(MSLAXIS axis);
    MSLAXIS GetActiveAxis() const { return active_axis; }
    virtual void AdvanceAxisTool()
    {
        switch (axis_tool) {
        case MSLTOOL::TOOL_TRANSLATE:
            axis_tool = MSLTOOL::TOOL_SCALE;
            break;
        case MSLTOOL::TOOL_SCALE:
            axis_tool = MSLTOOL::TOOL_ROTATE;
            break;
        case MSLTOOL::TOOL_ROTATE:
        default:
            axis_tool = MSLTOOL::TOOL_TRANSLATE;
            break;
        }
    }
    virtual void SetAxisTool(MSLTOOL mode) { axis_tool = mode; }
    MSLTOOL GetAxisTool() const { return axis_tool; }
    bool DragHandle(ModelPreview* preview, int mouseX, int mouseY, bool latch);
    void TranslateVector(glm::vec3& point) const;
    virtual int GetDefaultHandle() const { return CENTER_HANDLE; }
    virtual MSLTOOL GetDefaultTool() const { return MSLTOOL::TOOL_TRANSLATE; }
    virtual void MouseOverHandle(int handle);
    int GetNumSelectableHandles() const { return mSelectableHandles; }
    virtual bool IsXYTransHandle() const { return false; }
    virtual bool IsElevationHandle() const { return false; }
    bool GetSupportsZScaling() const { return supportsZScaling; }
    void SetSupportsZScaling(bool b) {
        supportsZScaling = b;
    }
    void CreateWithDepth(bool b) {
        createWithDepth = b;
    }
    void SetStartOnXAxis(bool b) {
        _startOnXAxis = b;
    }
    glm::vec3 GetWorldPosition() const { return glm::vec3(worldPos_x, worldPos_y, worldPos_z); }
    void SetWorldPosition(const glm::vec3& worldPos) { worldPos_x = worldPos.x; worldPos_y = worldPos.y; worldPos_z = worldPos.z; }
    glm::vec3 GetRotation() const { return glm::vec3(rotatex, rotatey, rotatez); }
    glm::quat GetRotationQuat() const { return rotate_quat; }
    glm::vec3 GetScaleMatrix() const { return glm::vec3(scalex, scaley, scalez); }
    void SetScaleMatrix(const glm::vec3& scale) const {
        scalex = scale.x; scaley = scale.y; scalez = scale.z;
    }
    glm::vec3 GetCenterPosition() const { return glm::vec3(GetHcenterPos(), GetVcenterPos(), GetDcenterPos()); }
    glm::vec3 GetActiveHandlePosition() const { return active_handle_pos; }
    virtual glm::vec3 GetHandlePosition(int handle) const;
    glm::vec3 GetRotationAngles() const { return angles; }
    glm::mat4 GetModelMatrix() const { return ModelMatrix; }

protected:
    ModelScreenLocation(int points);
    virtual ~ModelScreenLocation() {};
    virtual wxCursor CheckIfOverAxisHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle, float zoom, int scale) const;

    mutable float worldPos_x = 0.0f;
    mutable float worldPos_y = 0.0f;
    mutable float worldPos_z = 0.0f;
    mutable float scalex = 1.0f;
    mutable float scaley = 1.0f;
    mutable float scalez = 1.0f;
    float rotatex = 0.0f;
    float rotatey = 0.0f;
    float rotatez = 0.0f;
    mutable glm::mat4 ModelMatrix;
    mutable glm::mat4 TranslateMatrix;
    mutable glm::quat rotate_quat;
    mutable glm::vec3 aabb_min = glm::vec3(0.0f);
    mutable glm::vec3 aabb_max = glm::vec3(0.0f);

    // used for handle movement
    glm::vec3 saved_intersect = glm::vec3(0.0f);
    glm::vec3 saved_position = glm::vec3(0.0f);
    glm::vec3 saved_size = glm::vec3(0.0f);
    glm::vec3 saved_scale = glm::vec3(1.0f);
    glm::vec3 saved_rotate = glm::vec3(0.0);
    glm::vec3 drag_delta = glm::vec3(0.0);
    glm::vec3 angles = glm::vec3(0.0);

    mutable bool draw_3d = false;

    mutable std::vector<glm::vec3> handle_aabb_min;
    mutable std::vector<glm::vec3> handle_aabb_max;
    mutable std::vector<xlPoint> mHandlePosition;
    mutable glm::vec3 active_handle_pos = glm::vec3(0.0f);
    int mSelectableHandles = 0;
    bool _locked = false;
    int active_handle = -1;
    int highlighted_handle = -1;
    MSLAXIS active_axis = MSLAXIS::NO_AXIS;
    MSLTOOL axis_tool = MSLTOOL::TOOL_TRANSLATE;
    int tool_size = 1;
    bool supportsZScaling = false;
    bool createWithDepth =  false;
    bool _startOnXAxis = false;
    bool rotation_init = true;
    bool mouse_down = false;
};

//Default location that uses a bounding box - 4 corners and a rotate handle
class BoxedScreenLocation : public ModelScreenLocation {
public:
    BoxedScreenLocation();
    virtual ~BoxedScreenLocation() {}

    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;
    virtual MSLUPGRADE CheckUpgrade(wxXmlNode *node) override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const override;
    
    //new drawing code
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding) const override;

    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual bool Rotate(MSLAXIS axis, float factor) override;
    virtual bool Scale(const glm::vec3& factor) override;

    virtual void SelectHandle(int handle) override {}
    virtual int GetSelectedHandle() const override {return -1;}
    virtual int GetNumHandles() const override {return -1;}
    virtual void SelectSegment(int segment) override {}
    virtual int GetSelectedSegment() const override {return -1;}
    virtual bool HasCurve(int segment) const override {return false;}
    virtual void SetCurve(int segment, bool create = true) override {}
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) override {}
    virtual void InsertHandle(int after_handle, float zoom, int scale) override {}
    virtual void DeleteHandle(int handle) override {}
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;
    virtual void UpdateBoundingBox(float width, float height, float depth) override;

    virtual std::string GetDimension(float factor = 1.0) const override;
    virtual void AddDimensionProperties(wxPropertyGridInterface* grid, float factor = 1.0) const override;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual bool IsCenterBased() const override {return true;};

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) const override;

    virtual float GetHcenterPos() const override {
        return (float)worldPos_x;
    }
    virtual float GetVcenterPos() const override {
        return (float)worldPos_y;
    }
    virtual float GetDcenterPos() const override {
        return (float)worldPos_z;
    }
    virtual void SetHcenterPos(float f) override {
		worldPos_x = f;
    }
    virtual void SetVcenterPos(float f) override {
		worldPos_y = f;
    }
    virtual void SetDcenterPos(float f) override {
		worldPos_z = f;
    }

    virtual void SetPosition(float posx, float posy) override {
		worldPos_x = posx;
		worldPos_y = posy;
    }
    void SetScale(float x, float y) {
        scalex = x;
        scaley = y;
    }
    void SetScaleZ(float z) {
        scalez = z;
    }
    float GetScaleZ() const {
        return scalez;
    }

    void SetRotation(int r) {
        rotatez = r;
    }
    void SetPerspective2D(float p) {
        perspective = p;
    }

    virtual float GetTop() const override;
    virtual float GetLeft() const override;
    virtual float GetRight() const override;
    virtual float GetBottom() const override;
    virtual float GetFront() const override;
    virtual float GetBack() const override;
    virtual void SetTop(float i) override;
    virtual void SetLeft(float i) override;
    virtual void SetRight(float i) override;
    virtual void SetBottom(float i) override;
    virtual void SetFront(float i) override;
    virtual void SetBack(float i) override;
    virtual void SetMWidth(float w) override;
    virtual void SetMHeight(float h) override;
    virtual void SetMDepth(float d) override;
    virtual float GetMWidth() const override;
    virtual float GetMHeight() const override;
    virtual float GetMDepth() const override;
    virtual float GetRestorableMWidth() const override;
    virtual float GetRestorableMHeight() const override;
    virtual float GetRestorableMDepth() const override;

    int GetRotation() const {
        return rotatez;
    }

    float GetScaleX() { return scalex; }
    float GetScaleY() { return scaley; }

    virtual int GetDefaultHandle() const override { return CENTER_HANDLE; }
    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_SCALE; }
    float GetCentreX() const { return centerx; }
    float GetCentreY() const { return centery; }
    float GetCentreZ() const { return centerz; }
    void SetCentreX(float x) { centerx = x; worldPos_x = x; }
    void SetCentreY(float y) { centery = y; worldPos_y = y; }
    void SetCentreZ(float z) { centerz = z; worldPos_z = z; }

protected:
    BoxedScreenLocation(int points);
    float perspective;

    mutable float centerx;
    mutable float centery;
    mutable float centerz;
};

//Location that uses two points to define start/end
class TwoPointScreenLocation : public ModelScreenLocation {
public:
    TwoPointScreenLocation();
    virtual ~TwoPointScreenLocation();

    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;
    virtual MSLUPGRADE CheckUpgrade(wxXmlNode *node) override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float &x, float &y, float &z) const override;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const override;
    
    //new drawing code
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding) const override;
    virtual void DrawBoundingBox(xlVertexColorAccumulator *vac) const;

    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual bool Rotate(MSLAXIS axis, float factor) override;
    virtual bool Scale(const glm::vec3& factor) override;
    virtual void SelectHandle(int handle) override {}
    virtual int GetSelectedHandle() const override {return -1;}
    virtual int GetNumHandles() const override {return -1;}
    virtual void SelectSegment(int segment) override {}
    virtual int GetSelectedSegment() const override {return -1;}
    virtual bool HasCurve(int segment) const override {return false;}
    virtual void SetCurve(int segment, bool create = true) override {}
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) override {}
    virtual void InsertHandle(int after_handle, float zoom, int scale) override {}
    virtual void DeleteHandle(int handle) override {}
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;

    virtual void AddDimensionProperties(wxPropertyGridInterface* grid, float factor = 1.0) const override;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface* grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual std::string GetDimension(float factor = 1.0) const override;

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) const override;
    virtual float GetHcenterPos() const override;
    virtual float GetVcenterPos() const override;
    virtual float GetDcenterPos() const override;
    virtual void SetHcenterPos(float f) override;
    virtual void SetVcenterPos(float f) override;
    virtual void SetDcenterPos(float f) override;
    virtual bool IsCenterBased() const override {return false;};

    virtual void SetPosition(float posx, float posy) override;
    virtual float GetTop() const override;
    virtual float GetLeft() const override;
    virtual float GetRight() const override;
    virtual float GetBottom() const override;
    virtual float GetFront() const override;
    virtual float GetBack() const override;
    virtual void SetTop(float i) override;
    virtual void SetLeft(float i) override;
    virtual void SetRight(float i) override;
    virtual void SetBottom(float i) override;
    virtual void SetFront(float i) override;
    virtual void SetBack(float i) override;
    virtual void SetMWidth(float w) override;
    virtual void SetMHeight(float h) override;
    virtual float GetMWidth() const override;
    virtual float GetMHeight() const override;
    virtual float GetMDepth() const override;
    virtual void SetMDepth(float d) override;
    virtual void RotateAboutPoint(glm::vec3 position, glm::vec3 angle) override;
    void UpdateBoundingBox();

    virtual float GetYShear() const {return 0.0;}

    virtual int GetDefaultHandle() const override { return END_HANDLE; }
    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_TRANSLATE; }

    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;

    glm::vec3 GetPoint1() const { return origin; }
    glm::vec3 GetPoint2() const { return point2; }

protected:
    float x2 = 0.0f;
    float y2 = 0.0f;
    float z2 = 0.0f;
    mutable glm::vec3 origin;
    mutable glm::vec3 point2 = glm::vec3(0.0f);
    mutable glm::vec3 saved_point;
    mutable glm::vec3 center = glm::vec3(0.0f);
    mutable float length = 0;
    float saved_angle = 0.0f;
    bool minMaxSet = false;

    wxXmlNode *old = nullptr;
    mutable glm::mat4 matrix;
};

class ThreePointScreenLocation : public TwoPointScreenLocation {
public:
    ThreePointScreenLocation();
    virtual ~ThreePointScreenLocation();
    virtual void Read(wxXmlNode *node) override;
    virtual void Write(wxXmlNode *node) override;

    virtual void AddDimensionProperties(wxPropertyGridInterface* grid, float factor = 1.0) const override;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
    virtual wxCursor InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Node) override;
    virtual std::string GetDimension(float factor = 1.0) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding) const override;
    virtual void DrawBoundingBox(xlVertexColorAccumulator *vac) const override;

    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual float GetVScaleFactor() const override;
    virtual float GetYShear() const override;

    void SetHeight(float h) {
        height = h;
    }
    float GetHeight() const {
        return height;
    }
    virtual void SetMWidth(float w) override;
    virtual void SetMHeight(float h) override;
    virtual float GetMWidth() const override;
    virtual float GetMHeight() const override;
    void SetModelHandleHeight(bool b) {
        modelHandlesHeight = b;
    }
    void SetSupportsShear(bool b) {
        supportsShear = b;
    }
    void SetSupportsAngle(bool b) {
        supportsAngle = b;
    }
    void SetAngle(int a) {
        angle = a;
    }
    int GetAngle() const {
        return angle;
    }

    virtual int GetDefaultHandle() const override { return END_HANDLE; }
    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_TRANSLATE; }

    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;
    virtual void SetActiveAxis(MSLAXIS axis) override;
    virtual bool IsXYTransHandle() const override { return active_handle == SHEAR_HANDLE; }

private:
    bool modelHandlesHeight = false;
    bool supportsAngle = false;
    bool supportsShear = false;
    mutable glm::mat4 shearMatrix;
    float height = 1.0f;
    int angle = 0;
    float shear = 0.0f;
};

//Location that uses multiple points
class PolyPointScreenLocation : public ModelScreenLocation {
public:
    PolyPointScreenLocation();
    virtual ~PolyPointScreenLocation();

    virtual void Read(wxXmlNode* node) override;
    virtual void Write(wxXmlNode* node) override;
    virtual MSLUPGRADE CheckUpgrade(wxXmlNode* node) override;

    virtual void PrepareToDraw(bool is_3d, bool allow_selected) const override;
    virtual void TranslatePoint(float& x, float& y, float& z) const override;
    virtual void ApplyModelViewMatrices(xlGraphicsContext *ctx) const override;

    virtual bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const override;
    virtual bool HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const override;
    virtual bool HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const override;
    virtual wxCursor CheckIfOverHandles(ModelPreview* preview, int& handle, int x, int y) const override;
    virtual wxCursor CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int& handle, float zoom, int scale) const override;

    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale) const override;
    virtual bool DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding) const override;

    virtual int MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) override;
    virtual int MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) override;
    virtual bool Rotate(MSLAXIS axis, float factor) override;
    virtual bool Scale(const glm::vec3& factor) override;
    virtual void SelectHandle(int handle) override;
    virtual int GetSelectedHandle() const override { return selected_handle; }
    virtual int GetNumHandles() const override { return num_points; }
    virtual void SelectSegment(int segment) override;
    virtual int GetSelectedSegment() const override { return selected_segment; }
    virtual bool HasCurve(int segment) const override { return mPos[segment].has_curve; }
    virtual void SetCurve(int seg_num, bool create = true) override;
    virtual void AddHandle(ModelPreview* preview, int mouseX, int mouseY) override;
    virtual void InsertHandle(int after_handle, float zoom, int scale) override;
    virtual void DeleteHandle(int handle) override;
    virtual wxCursor InitializeLocation(int& handle, int x, int y, const std::vector<NodeBaseClassPtr>& Nodes, ModelPreview* preview) override;
    virtual void UpdateBoundingBox(const std::vector<NodeBaseClassPtr>& Node) override;

    virtual void AddDimensionProperties(wxPropertyGridInterface* grid, float factor = 1.0) const override;
    virtual void AddSizeLocationProperties(wxPropertyGridInterface* grid) const override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    virtual std::string GetDimension(float factor = 1.0) const override;

    virtual glm::vec2 GetScreenOffset(ModelPreview* preview) const override;
    virtual float GetHcenterPos() const override;
    virtual float GetVcenterPos() const override;
    virtual float GetDcenterPos() const override;
    virtual void SetHcenterPos(float f) override;
    virtual void SetVcenterPos(float f) override;
    virtual void SetDcenterPos(float f) override;
    virtual bool IsCenterBased() const override { return false; };

    virtual void SetPosition(float posx, float posy) override;
    virtual float GetTop() const override;
    virtual float GetLeft() const override;
    virtual float GetRight() const override;
    virtual float GetBottom() const override;
    virtual float GetFront() const override;
    virtual float GetBack() const override;
    virtual void SetTop(float i) override;
    virtual void SetLeft(float i) override;
    virtual void SetRight(float i) override;
    virtual void SetBottom(float i) override;
    virtual void SetFront(float i) override;
    virtual void SetBack(float i) override;
    virtual void SetMWidth(float w) override;
    virtual void SetMHeight(float h) override;
    virtual void SetMDepth(float d) override;
    virtual float GetMWidth() const override;
    virtual float GetMHeight() const override;
    virtual float GetMDepth() const override;
    virtual void RotateAboutPoint(glm::vec3 position, glm::vec3 angle) override;

    virtual int GetDefaultHandle() const override { return END_HANDLE; }
    virtual MSLTOOL GetDefaultTool() const override { return MSLTOOL::TOOL_XY_TRANS; }
    virtual float GetYShear() const { return 0.0; }
    virtual void SetActiveHandle(int handle) override;
    virtual void AdvanceAxisTool() override;
    virtual void SetAxisTool(MSLTOOL mode) override;
    virtual void SetActiveAxis(MSLAXIS axis) override;

protected:
    struct xlPolyPoint {
        float x;
        float y;
        float z;
        mutable xlPoint cp0;
        mutable xlPoint cp1;
        mutable bool has_curve;
        mutable BezierCurveCubic3D* curve;
        mutable glm::mat4* matrix;
        mutable glm::mat4* mod_matrix;
        mutable float seg_scale;

        glm::vec3 AsVector() const { return glm::vec3(x, y, z); }
    };
    mutable std::vector<xlPolyPoint> mPos;
    int num_points = 0;
    int selected_handle = 0;
    mutable std::mutex _mutex;
    mutable float minX = 0.0f;
    mutable float minY = 0.0f;
    mutable float maxX = 0.0f;
    mutable float maxY = 0.0f;
    mutable float minZ = 0.0f;
    mutable float maxZ = 0.0f;
    mutable int selected_segment;
    mutable std::vector<glm::vec3> seg_aabb_min;
    mutable std::vector<glm::vec3> seg_aabb_max;
    mutable glm::mat4 main_matrix;
    mutable glm::vec3 saved_point;
    mutable glm::vec3 center;
    mutable glm::vec3 rotate_pt;
    float saved_angle = 0.0f;
    void FixCurveHandles();
    void AdjustAllHandles(glm::mat4& mat);
};

