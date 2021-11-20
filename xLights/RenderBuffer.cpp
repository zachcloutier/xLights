
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <cmath>
#ifdef _MSC_VER
	// required so M_PI will be defined by MSC
	#define _USE_MATH_DEFINES
	#include <math.h>
#endif

#include "RenderBuffer.h"
#include "sequencer/Effect.h"
#include "xLightsMain.h"
#include "xLightsXmlFile.h"
#include "UtilFunctions.h"
#include "models/DMX/DmxModel.h"
#include "models/DMX/DmxColorAbility.h"
#include "GPURenderUtils.h"

#include <log4cpp/Category.hh>
#include "Parallel.h"

template <class CTX>
class ContextPool {
public:
    
    ContextPool(std::function<CTX* ()> alloc, std::string type = ""): allocator(alloc), _type(type) {
    }
    ~ContextPool() {
        while (!contexts.empty()) {
            CTX *ret = contexts.front();
            delete ret;
            contexts.pop();
        }
    }
    
    CTX *GetContext() {
        // This seems odd but manually releasing the lock causes hard crashes on Visual Studio
        bool contextsEmpty = false;
        {
            std::unique_lock<std::mutex> locker(lock);
            contextsEmpty = contexts.empty();
        }

        if (contextsEmpty) {
            return allocator();
        }

        {
            std::unique_lock<std::mutex> locker(lock);
            CTX *ret = contexts.front();
            contexts.pop();
            return ret;
        }
    }
    void ReleaseContext(CTX *pctx) {
        std::unique_lock<std::mutex> locker(lock);
        contexts.push(pctx);
    }
    
private:
    std::mutex lock;
    std::queue<CTX*> contexts;
    std::function<CTX* ()> allocator;
    std::string _type;
};

static ContextPool<TextDrawingContext> *TEXT_CONTEXT_POOL = nullptr;
static ContextPool<PathDrawingContext> *PATH_CONTEXT_POOL = nullptr;

void DrawingContext::Initialize(wxWindow *parent) {
    if (TEXT_CONTEXT_POOL == nullptr) {
        TEXT_CONTEXT_POOL = new ContextPool<TextDrawingContext>([parent]() {
            if (wxThread::IsMain()) {
                return new TextDrawingContext(10, 10 ,false);
            } else {
                std::mutex mtx;
                std::condition_variable signal;
                std::unique_lock<std::mutex> lck(mtx);
                TextDrawingContext *tdc;
                parent->CallAfter([&mtx, &signal, &tdc]() {
                    std::unique_lock<std::mutex> lck(mtx);
                    tdc = new TextDrawingContext(10, 10 ,false);
                    signal.notify_all();
                });
                signal.wait(lck);
                return tdc;
            }
        });
    }
    if (PATH_CONTEXT_POOL == nullptr) {
        PATH_CONTEXT_POOL = new ContextPool<PathDrawingContext>([parent]() {
            if (wxThread::IsMain()) {
                return new PathDrawingContext(10, 10 ,false);
            } else {
                std::mutex mtx;
                std::condition_variable signal;
                std::unique_lock<std::mutex> lck(mtx);
                PathDrawingContext *tdc;
                parent->CallAfter([&mtx, &signal, &tdc]() {
                    std::unique_lock<std::mutex> lck(mtx);
                    tdc = new PathDrawingContext(10, 10 ,false);
                    signal.notify_all();
                });
                signal.wait(lck);
                return tdc;
            }
        });
    }
}

void DrawingContext::CleanUp() {
    if (TEXT_CONTEXT_POOL != nullptr) {
        delete TEXT_CONTEXT_POOL;
        TEXT_CONTEXT_POOL = nullptr;
    }
    if (PATH_CONTEXT_POOL != nullptr) {
        delete PATH_CONTEXT_POOL;
        PATH_CONTEXT_POOL = nullptr;
    }
}

PathDrawingContext* PathDrawingContext::GetContext() {
    if (PATH_CONTEXT_POOL != nullptr) {
        return PATH_CONTEXT_POOL->GetContext();
    }
    return nullptr;
}

void PathDrawingContext::ReleaseContext(PathDrawingContext* pdc) {
    if (PATH_CONTEXT_POOL != nullptr) {
        return PATH_CONTEXT_POOL->ReleaseContext(pdc);
    }
}

TextDrawingContext* TextDrawingContext::GetContext() {
    if (TEXT_CONTEXT_POOL != nullptr) {
        return TEXT_CONTEXT_POOL->GetContext();
    }
    return nullptr;
}

void TextDrawingContext::ReleaseContext(TextDrawingContext* pdc) {
    if (TEXT_CONTEXT_POOL != nullptr) {
        return TEXT_CONTEXT_POOL->ReleaseContext(pdc);
    }
}

EffectRenderCache::EffectRenderCache() {}
EffectRenderCache::~EffectRenderCache() {}
void RenderBuffer::SetAllowAlphaChannel(bool a) { allowAlpha = a; }
void RenderBuffer::SetFrameTimeInMs(int i) { frameTimeInMs = i; }

inline void unshare(wxObject &o) {
    if (o.GetRefData() != nullptr) {
        o.UnShare();
    }
}

inline void unshare(const wxObject &o2) {
    wxObject *o = (wxObject*)&o2;
    if (o->GetRefData() != nullptr) {
        o->UnShare();
    }
}

AudioManager* RenderBuffer::GetMedia() const
{
	if (xLightsFrame::CurrentSeqXmlFile == nullptr)
	{
		return nullptr;
	}
	return xLightsFrame::CurrentSeqXmlFile->GetMedia();
}

Model* RenderBuffer::GetModel() const
{
    // this only returns a model or model group
    if (cur_model.find("/") != std::string::npos) {
        return nullptr;
    }
    return frame->AllModels[cur_model];
}

Model* RenderBuffer::GetPermissiveModel() const
{
    // This will return models, model groups or submodels and strands
    return frame->AllModels.GetModel(cur_model);
}

std::string RenderBuffer::GetModelName() const
{
    Model* m = GetPermissiveModel();

    if (m != nullptr)
    {
        return m->GetFullName();
    }

    return cur_model;
}

void RenderBuffer::AlphaBlend(const RenderBuffer& src)
{
    if (src.BufferWi != BufferWi || src.BufferHt != BufferHt) return;

    parallel_for(0, GetPixelCount(), [&src, this](int idx) {
        const auto &pnew = src.pixels[idx];
        auto &pold = pixels[idx];
        if (pnew.alpha == 255 || pold == xlBLACK) {
            pold = pnew;
        } else if (pnew.alpha > 0 && pnew != xlBLACK) {
            xlColor c;
            int r = pnew.red + pold.red * (255 - pnew.alpha) / 255;
            if (r > 255) r = 255;
            pold.red = r;
            int g = pnew.green + pold.green * (255 - pnew.alpha) / 255;
            if (g > 255) g = 255;
            pold.green = g;
            int b = pnew.blue + pold.blue * (255 - pnew.alpha) / 255;
            if (b > 255) b = 255;
            pold.blue = b;
            int a = pnew.alpha + pold.alpha * (255 - pnew.alpha) / 255;
            if (a > 255) a = 255;
            pold.alpha = a;
        }
    }, 2000);
}

inline double DegToRad(double deg) { return (deg * M_PI) / 180.0; }

DrawingContext::DrawingContext(int BufferWi, int BufferHt, bool allowShared, bool alpha) : nullBitmap(wxNullBitmap)
{
    gc = nullptr;
    dc = nullptr;
    unshare(nullBitmap);
    image = new wxImage(BufferWi > 0 ? BufferWi : 1, BufferHt > 0 ? BufferHt : 1);
    if (alpha) {
        image->SetAlpha();
        for(wxCoord x=0; x<BufferWi; x++) {
            for(wxCoord y=0; y<BufferHt; y++) {
                image->SetAlpha(x, y, wxIMAGE_ALPHA_TRANSPARENT);
            }
        }
    }
    bitmap = new wxBitmap(*image);
    dc = new wxMemoryDC(*bitmap);

    if (!allowShared) {
        //make sure we UnShare everything that is being held onto
        //also use "non-normal" defaults to avoid "==" issue that
        //would keep it from using the non-shared versions
        wxFont font(*wxITALIC_FONT);
        unshare(font);
        dc->SetFont(font);

        wxBrush brush(*wxYELLOW_BRUSH);
        unshare(brush);
        dc->SetBrush(brush);
        dc->SetBackground(brush);

        wxPen pen(*wxGREEN_PEN);
        unshare(pen);
        dc->SetPen(pen);

        unshare(dc->GetBrush());
        unshare(dc->GetBackground());
        unshare(dc->GetFont());
        unshare(dc->GetPen());
        unshare(dc->GetTextForeground());
        unshare(dc->GetTextBackground());
    #ifndef LINUX
        wxColor c(12, 25, 3);
        unshare(c);
        dc->SetTextBackground(c);

        wxColor c2(0, 35, 5);
        unshare(c2);
        dc->SetTextForeground(c2);
    #endif
    }

    dc->SelectObject(nullBitmap);
    delete bitmap;
    bitmap = nullptr;
}

DrawingContext::~DrawingContext() {
    if (gc != nullptr) {
        delete gc;
    }
    if (dc != nullptr) {
        delete dc;
    }
    if (bitmap != nullptr) {
        delete bitmap;
    }
    if (image != nullptr) {
        delete image;
    }
}


PathDrawingContext::PathDrawingContext(int BufferWi, int BufferHt, bool allowShared)
    : DrawingContext(BufferWi, BufferHt, allowShared, true) {}

PathDrawingContext::~PathDrawingContext() {}

TextDrawingContext::TextDrawingContext(int BufferWi, int BufferHt, bool allowShared)
#ifdef __WXMSW__
    : DrawingContext(BufferWi, BufferHt, allowShared, true)
#elif defined(__WXOSX__)
    : DrawingContext(BufferWi, BufferHt, allowShared, true)
#elif defined(LINUX)
    // Linux does text rendering on main thread so using the shared stuff is fine
    : DrawingContext(BufferWi, BufferHt, true, true)
#endif
{
    fontStyle = 0;
    fontSize = 0;
}

TextDrawingContext::~TextDrawingContext() {}

void DrawingContext::ResetSize(int BufferWi, int BufferHt) {
    if (bitmap != nullptr) {
        delete bitmap;
        bitmap = nullptr;
    }
    if (image != nullptr) {
        delete image;
    }
    image = new wxImage(BufferWi > 0 ? BufferWi : 1, BufferHt > 0 ? BufferHt : 1);
    if (AllowAlphaChannel()) {
        image->SetAlpha();
        for(wxCoord x=0; x<BufferWi; x++) {
            for(wxCoord y=0; y<BufferHt; y++) {
                image->SetAlpha(x, y, wxIMAGE_ALPHA_TRANSPARENT);
            }
        }
    }
}

void DrawingContext::Clear() {
    if (dc != nullptr)
    {
        dc->SelectObject(nullBitmap);
        if (bitmap != nullptr) {
            delete bitmap;
        }
        image->Clear();

        if (AllowAlphaChannel()) {
            image->SetAlpha();
            memset(image->GetAlpha(), wxIMAGE_ALPHA_TRANSPARENT, image->GetWidth() * image->GetHeight());
            for (wxCoord x = 0; x < image->GetWidth(); x++) {
                for (wxCoord y = 0; y < image->GetHeight(); y++) {
                    image->SetAlpha(x, y, wxIMAGE_ALPHA_TRANSPARENT);
                }
            }
            bitmap = new wxBitmap(*image, 32);
        }
        else {
            bitmap = new wxBitmap(*image);
        }
        dc->SelectObject(*bitmap);
    }
}

void PathDrawingContext::Clear() {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (gc != nullptr) {
        delete gc;
        gc = nullptr;
    }
    DrawingContext::Clear();
    gc = wxGraphicsContext::Create(*dc);

    if (gc == nullptr)
    {
        logger_base.error("PathDrawingContext DC creation failed.");
        return;
    }

    gc->SetAntialiasMode(wxANTIALIAS_NONE);
    gc->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_FAST);
    gc->SetCompositionMode(wxCompositionMode::wxCOMPOSITION_SOURCE);
}

void TextDrawingContext::Clear()
{

    if (gc != nullptr) {
        delete gc;
        gc = nullptr;
    }

    DrawingContext::Clear();

#ifdef __WXMSW__
    // must use the Direct2D renderer to get the color emoji's
    // along with custom build of wxWidgets until
    // https://trac.wxwidgets.org/ticket/19275#ticket
    // is applied/fixed
    static std::mutex singleThreadContextCreate;
    {
        // we need to lock before we do this otherwise we try to load libraries concurrently on many threads which causes problems
        std::unique_lock<std::mutex> locker(singleThreadContextCreate);
        gc = wxGraphicsRenderer::GetDirect2DRenderer()->CreateContext(*dc);
        if (gc != nullptr) {
            gc->SetPen(wxPen()); // we set a pen just to ensure the renderer is initialised ... if we dont do this there are scenarios where deleting it can run into issues
        }
    }
#else
    gc = wxGraphicsContext::Create(*dc);
#endif

    if (gc == nullptr) {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.error("TextDrawingContext DC creation failed.");
        return;
    }

    gc->SetAntialiasMode(wxANTIALIAS_NONE);
    gc->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_FAST);
    gc->SetCompositionMode(wxCompositionMode::wxCOMPOSITION_SOURCE);
}

void TextDrawingContext::SetOverlayMode(bool b) {
    gc->SetCompositionMode(b ? wxCompositionMode::wxCOMPOSITION_OVER : wxCompositionMode::wxCOMPOSITION_SOURCE);
}

bool TextDrawingContext::AllowAlphaChannel() {
    return true;
}

wxImage *DrawingContext::FlushAndGetImage() {
    if (gc != nullptr) {
        gc->Flush();
        delete gc;
        gc = nullptr;
    }
    dc->SelectObject(nullBitmap);
    *image = bitmap->ConvertToImage();
    dc->SelectObject(*bitmap);
    return image;
}

void PathDrawingContext::SetPen(wxPen &pen) {
    if (gc != nullptr)
        gc->SetPen(pen);
}

void TextDrawingContext::SetPen(wxPen &pen) {
    if (gc != nullptr) {
        gc->SetPen(pen);
    } else {
        dc->SetPen(pen);
    }
}

wxGraphicsPath PathDrawingContext::CreatePath()
{
    return gc->CreatePath();
}

void PathDrawingContext::StrokePath(wxGraphicsPath& path)
{
    gc->StrokePath(path);
}

void TextDrawingContext::SetFont(wxFontInfo &font, const xlColor &color) {
    if (gc != nullptr) {
        int style = wxFONTFLAG_NOT_ANTIALIASED;
        if (font.GetWeight() == wxFONTWEIGHT_BOLD) {
            style |= wxFONTFLAG_BOLD;
        }
        if (font.GetWeight() == wxFONTWEIGHT_LIGHT) {
            style |= wxFONTFLAG_LIGHT;
        }
        if (font.GetStyle() == wxFONTSTYLE_ITALIC) {
            style |= wxFONTFLAG_ITALIC;
        }
        if (font.GetStyle() == wxFONTSTYLE_SLANT) {
            style |= wxFONTFLAG_SLANT;
        }
        if (font.IsUnderlined()) {
            style |= wxFONTFLAG_UNDERLINED;
        }
        if (font.IsStrikethrough()) {
            style |= wxFONTFLAG_STRIKETHROUGH;
        }

        if (style != fontStyle
            || font.GetPixelSize().y != fontSize
            || font.GetFaceName() != fontName
            || color != fontColor) {
            this->font = gc->CreateFont(font.GetPixelSize().y, font.GetFaceName(), style, color.asWxColor());

#ifdef __WXMSW__
            // if font is not true type then we get a null font and bad things happen so at least set it to a valid font ... Somehow we really should not allow the selection of invalid fonts
            if (this->font.IsNull()) {
                this->font = gc->CreateFont(font.GetPixelSize().y, "segoe ui", style, color.asWxColor());
            }
#endif

            fontStyle = style;
            fontSize = font.GetPixelSize().y;
            fontName = font.GetFaceName();
            fontColor = color;
        }
        gc->SetFont(this->font);
    } else {
        wxFont f(font);
    #ifdef __WXMSW__
        /*
         Here is the format for NativeFontInfo on Windows (taken from the source)
         We want to change lfQuality from 2 to 3 - this disables antialiasing
         s.Printf(wxS("%d;%ld;%ld;%ld;%ld;%ld;%d;%d;%d;%d;%d;%d;%d;%d;%s"),
         0, // version, in case we want to change the format later
         lf.lfHeight,
         lf.lfWidth,
         lf.lfEscapement,
         lf.lfOrientation,
         lf.lfWeight,
         lf.lfItalic,
         lf.lfUnderline,
         lf.lfStrikeOut,
         lf.lfCharSet,
         lf.lfOutPrecision,
         lf.lfClipPrecision,
         lf.lfQuality,
         lf.lfPitchAndFamily,
         lf.lfFaceName);*/
        wxString s = f.GetNativeFontInfoDesc();
        s.Replace(";2;",";3;",false);
        f.SetNativeFontInfo(s);
    #endif
        dc->SetFont(f);
        dc->SetTextForeground(color.asWxColor());
    }
}

void TextDrawingContext::DrawText(const wxString &msg, int x, int y, double rotation) {
    if (gc != nullptr) {
        gc->DrawText(msg, x, y, DegToRad(rotation));
    } else {
        dc->DrawRotatedText(msg, x, y, rotation);
    }
}

void TextDrawingContext::DrawText(const wxString &msg, int x, int y) {
    if (gc != nullptr) {
        gc->DrawText(msg, x, y);
    } else {
        dc->DrawText(msg, x, y);
    }
}

void TextDrawingContext::GetTextExtent(const wxString &msg, double *width, double *height) {
    if (gc != nullptr) {
        gc->GetTextExtent(msg, width, height);
    } else {
        wxSize size = dc->GetTextExtent(msg);
        *width = size.GetWidth();
        *height = size.GetHeight();
    }
}
void TextDrawingContext::GetTextExtents(const wxString &msg, wxArrayDouble &extents) {
    if (gc != nullptr) {
        gc->GetPartialTextExtents(msg, extents);
        return;
    }
    wxArrayInt sizes;
    dc->GetPartialTextExtents(msg, sizes);
    extents.resize(sizes.size());
    for (int x = 0; x < sizes.size(); x++) {
        extents[x] = sizes[x];
    }
}


RenderBuffer::RenderBuffer(xLightsFrame *f) : frame(f)
{
    BufferHt = 0;
    BufferWi = 0;
    ModelBufferHt = 0;
    ModelBufferWi = 0;
    curPeriod = 0;
    curEffStartPer = 0;
    curEffEndPer = 0;
    fadeinsteps = 0;
    fadeoutsteps = 0;
    allowAlpha = false;
    dmx_buffer = false;
    needToInit = true;
    _nodeBuffer = false;
    frameTimeInMs = 50;
    _textDrawingContext = nullptr;
    _pathDrawingContext = nullptr;
    tempInt = tempInt2 = 0;
    isTransformed = false;
}

RenderBuffer::~RenderBuffer()
{    
    if (_isCopy) Forget();

    //dtor
    if (_textDrawingContext != nullptr) {
        TextDrawingContext::ReleaseContext(_textDrawingContext);
    }
    if (_pathDrawingContext != nullptr) {
        PathDrawingContext::ReleaseContext(_pathDrawingContext);
    }
    for (auto& it : infoCache) {
        delete it.second;
        it.second = nullptr;
    }
    if (gpuRenderData) {
        GPURenderUtils::cleanUp(this);
        gpuRenderData = nullptr;
    }
}

PathDrawingContext * RenderBuffer::GetPathDrawingContext()
{
    if (_pathDrawingContext == nullptr)
    {
        _pathDrawingContext = PathDrawingContext::GetContext();
        _pathDrawingContext->ResetSize(BufferWi, BufferHt);
    }

    return _pathDrawingContext; 
}

TextDrawingContext * RenderBuffer::GetTextDrawingContext()
{
    if (_textDrawingContext == nullptr)
    {
        _textDrawingContext = TextDrawingContext::GetContext();
        _textDrawingContext->ResetSize(BufferWi, BufferHt);
    }

    return _textDrawingContext;
}

void RenderBuffer::InitBuffer(int newBufferHt, int newBufferWi, int newModelBufferHt, int newModelBufferWi, const std::string& bufferTransform, bool nodeBuffer)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_pathDrawingContext != nullptr && (BufferHt != newBufferHt || BufferWi != newBufferWi)) {
        _pathDrawingContext->ResetSize(newBufferWi, newBufferHt);
    }
    if (_textDrawingContext != nullptr && (BufferHt != newBufferHt || BufferWi != newBufferWi)) {
        _textDrawingContext->ResetSize(newBufferWi, newBufferHt);
    }
    _nodeBuffer = nodeBuffer;
    BufferHt = newBufferHt;
    BufferWi = newBufferWi;
    ModelBufferHt = newModelBufferHt;
    ModelBufferWi = newModelBufferWi;
    if (ModelBufferHt * ModelBufferWi < std::max(BufferHt, ModelBufferHt) * std::max(BufferWi, ModelBufferWi)) {
        wxASSERT(false);
        logger_base.warn("RenderBuffer had to be expanded for %s from %d to %d pixels", (const char *)GetModelName().c_str(), ModelBufferHt * ModelBufferWi, std::max(BufferHt, ModelBufferHt) * std::max(BufferWi, ModelBufferWi));
    }
    size_t NumPixels = std::max(BufferHt, ModelBufferHt) * std::max(BufferWi, ModelBufferWi);
    // This is an absurdly high number but there are circumstances right now when creating a buffer based on a zoomed in camera when these can be hit.
    //wxASSERT(NumPixels < 500000);
    
    pixelVector.resize(NumPixels);
    pixels = &pixelVector[0];
    tempbufVector.resize(NumPixels);
    tempbuf = &tempbufVector[0];
    isTransformed = (bufferTransform != "None");
}

void RenderBuffer::Clear()
{
    if (pixelVector.size() > 0) {
        memset(pixels, 0x00, sizeof(xlColor) * pixelVector.size());
    }
}

void RenderBuffer::SetPalette(xlColorVector& newcolors, xlColorCurveVector& newcc)
{
    palette.Set(newcolors, newcc);
}

size_t RenderBuffer::GetColorCount()
{
    return palette.Size();
}

class SinTable {
public:
    static constexpr float precision = 300.0f; // gradations per Pi, 942 entries of size float is under 4K or less than a memory page
    static constexpr int modulus = (int)(M_PI * precision) + 1;
    static constexpr int modulus2 = modulus * 2;

    SinTable() {
        for (int i = 0; i<modulus; i++) {
            float f = i;
            f /= precision;
            table[i]=sinf(f);
        }
        /*
        for (int x = -720; x <= 720; x++) {
            float s = ((float)x)*3.14159f/180.0f;
            printf("%d:\t%f\t%f\n", x, this->cos(s), cosf(s));
        }
         */
    }
    ~SinTable() {
    }
    float table[modulus]; // lookup table

    float sinLookup(int a) {
        if (a >= 0) {
            int idx = a%(modulus2);
            if (idx >= modulus) {
                idx -= modulus;
                return -table[idx];
            }
            return table[idx];
        }
        int idx = -a%(modulus2);
        if (idx >= modulus) {
            idx -= modulus;
            return table[idx];
        }
        return -table[idx];
    }
    float sin(float rad) {
        float idx = rad * precision + 0.5f;
        return sinLookup((int)idx);
    }
    float cos(float a) {
        return this->sin(a + M_PI_2);
    }
};
static SinTable sinTable;

float RenderBuffer::sin(float rad)
{
    return sinTable.sin(rad);
}
float RenderBuffer::cos(float rad)
{
    return sinTable.cos(rad);
}

// generates a random number between num1 and num2 inclusive
double RenderBuffer::RandomRange(double num1, double num2) const
{
    double hi,lo;
    if (num1 < num2)
    {
        lo = num1;
        hi = num2;
    }
    else
    {
        lo = num2;
        hi = num1;
    }
    return rand01()*(hi-lo)+ lo;
}

void RenderBuffer::Color2HSV(const xlColor& color, HSVValue& hsv) const
{
    color.toHSV(hsv);
}

// sets newcolor to a random color between hsv1 and hsv2
void RenderBuffer::SetRangeColor(const HSVValue& hsv1, const HSVValue& hsv2, HSVValue& newhsv)
{
    newhsv.hue=RandomRange(hsv1.hue,hsv2.hue);
    newhsv.saturation=RandomRange(hsv1.saturation,hsv2.saturation);
    newhsv.value=1.0;
}

// return a value between c1 and c2
uint8_t RenderBuffer::ChannelBlend(uint8_t c1, uint8_t c2, float ratio)
{
    return c1 + floor(ratio * (c2 - c1) + 0.5);
}

void RenderBuffer::Get2ColorBlend(int coloridx1, int coloridx2, float ratio, xlColor &color)
{
    color = palette.GetColor(coloridx1);
    const xlColor &c2 = palette.GetColor(coloridx2);
    Get2ColorBlend(color, c2, ratio);
}

void RenderBuffer::Get2ColorBlend(xlColor& color, xlColor color2, float ratio)
{
    color.Set(ChannelBlend(color.Red(), color2.Red(), ratio), ChannelBlend(color.Green(), color2.Green(), ratio), ChannelBlend(color.Blue(), color2.Blue(), ratio));
}

void RenderBuffer::Get2ColorAlphaBlend(const xlColor& c1, const xlColor& c2, float ratio, xlColor &color)
{
    color.Set(ChannelBlend(c1.Red(),c2.Red(),ratio), ChannelBlend(c1.Green(),c2.Green(),ratio), ChannelBlend(c1.Blue(),c2.Blue(),ratio));
}

inline uint8_t SumUInt8(uint8_t c1, uint8_t c2)
{
    int x = c1;
    x += c2;
    if (x > 255) x = 255;
    return x;
}

HSVValue RenderBuffer::Get2ColorAdditive(HSVValue& hsv1, HSVValue& hsv2) const
{
    xlColor rgb;
    xlColor rgb1(hsv1);
    xlColor rgb2(hsv2);
    rgb.red = SumUInt8(rgb1.red, rgb2.red);
    rgb.green = SumUInt8(rgb1.green, rgb2.green);
    rgb.blue = SumUInt8(rgb1.blue, rgb2.blue);
    return rgb.asHSV();
}
// 0 <= n < 1
void RenderBuffer::GetMultiColorBlend(float n, bool circular, xlColor &color, int reserveColours)
{
    size_t colorcnt = GetColorCount() - reserveColours;
    if (colorcnt <= 1)
    {
        palette.GetColor(0, color);
        return;
    }

    if (n >= 1.0) n = 0.99999f;
    if (n < 0.0) n = 0.0f;
    float realidx = circular ? n * colorcnt : n * (colorcnt - 1);
    int coloridx1 = floor(realidx);
    int coloridx2 = (coloridx1 + 1) % colorcnt;
    float ratio = realidx - float(coloridx1);
    Get2ColorBlend(coloridx1, coloridx2, ratio, color);
}


// 0,0 is lower left
void RenderBuffer::SetPixel(int x, int y, const xlColor &color, bool wrap, bool useAlpha, bool dmx_ignore)
{
    if (!dmx_ignore && dmx_buffer) {
        SetPixelDMXModel(x, y, color);
        return;
    }
        
    if (wrap) {
        while (x < 0) {
            x += BufferWi;
        }
        while (y < 0) {
            y += BufferHt;
        }
        while (x > BufferWi) {
            x -= BufferWi;
        }
        while (y > BufferHt) {
            y -= BufferHt;
        }
    }
    
    // I dont like this ... it should actually never happen
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && y*BufferWi + x < pixelVector.size())
    {
        // if you do this sparkles dont work when 100% transparent on effect ... so dont do it
        //if (color.alpha == 0)
        //{
            // transparent ... dont do anything
        //}
        //else 
        if (useAlpha && color.Alpha() != 255)
        {
            xlColor pnew = color;
            xlColor pold = pixels[y*BufferWi + x];

            xlColor c;
            int r = pnew.red + pold.red * (255 - pnew.alpha) / 255;
            if (r > 255) r = 255;
            c.red = r;
            int g = pnew.green + pold.green * (255 - pnew.alpha) / 255;
            if (g > 255) g = 255;
            c.green = g;
            int b = pnew.blue + pold.blue * (255 - pnew.alpha) / 255;
            if (b > 255) b = 255;
            c.blue = b;
            int a = pnew.alpha + pold.alpha * (255 - pnew.alpha) / 255;
            if (a > 255) a = 255;
            c.alpha = a;

            pixels[y*BufferWi + x] = c;
        }
        else
        {
            pixels[y*BufferWi + x] = color;
        }
    }
}

void RenderBuffer::ProcessPixel(int x_pos, int y_pos, const xlColor &color, bool wrap_x, bool wrap_y)
{
    int x_value = x_pos;
    if (wrap_x)  // if set wrap image at boundary
    {
        x_value %= BufferWi;
        x_value = (x_value >= 0) ? (x_value) : (BufferWi + x_value);
    }
    int y_value = y_pos;
    if (wrap_y)
    {
        y_value %= BufferHt;
        y_value = (y_value >= 0) ? (y_value) : (BufferHt + y_value);
    }
    SetPixel(x_value, y_value, color);
}

// 0,0 is lower left
void RenderBuffer::SetPixel(int x, int y, const HSVValue& hsv, bool wrap)
{
    if (dmx_buffer) {
        SetPixelDMXModel(x, y, xlColor(hsv));
        return;
    }

    if (wrap) {
        while (x < 0) {
            x += BufferWi;
        }
        while (y < 0) {
            y += BufferHt;
        }
        while (x > BufferWi) {
            x -= BufferWi;
        }
        while (y > BufferHt) {
            y -= BufferHt;
        }
    }
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && y*BufferWi + x < pixelVector.size())
    {
        pixels[y*BufferWi+x] = hsv;
    }
}

void RenderBuffer::SetNodePixel(int nodeNum, const xlColor &color, bool dmx_ignore) {
    if (nodeNum < Nodes.size()) {
        for (auto &a : Nodes[nodeNum]->Coords) {
            SetPixel(a.bufX, a.bufY, color, false, false, dmx_ignore);
        }
    }
}

void RenderBuffer::CopyNodeColorsToPixels(std::vector<bool> &done) {
    parallel_for(0, Nodes.size(), [&](int n) {
        xlColor c;
        Nodes[n]->GetColor(c);
        for (auto &a : Nodes[n]->Coords) {
            int x = a.bufX;
            int y = a.bufY;
            if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && y*BufferWi + x < pixelVector.size()) {
                pixels[y*BufferWi+x] = c;
                done[y*BufferWi+x] = true;
            }
        }

    }, 500);
}


//copy src to dest: -DJ
void RenderBuffer::CopyPixel(int srcx, int srcy, int destx, int desty)
{
    if ((srcx >= 0) && (srcx < BufferWi) && (srcy >= 0) && (srcy < BufferHt) && srcy*BufferWi + srcx < pixelVector.size())
        if ((destx >= 0) && (destx < BufferWi) && (desty >= 0) && (desty < BufferHt) && desty*BufferWi + destx < pixelVector.size())
        {
            pixels[desty * BufferWi + destx] = pixels[srcy * BufferWi + srcx];
        }
}

void RenderBuffer::DrawHLine(int y, int xstart, int xend, const xlColor &color, bool wrap) {
    if (xstart > xend) {
        int i = xstart;
        xstart = xend;
        xend = i;
    }
    for (int x = xstart; x <= xend; x++) {
        SetPixel(x, y, color, wrap);
    }
}
void RenderBuffer::DrawVLine(int x, int ystart, int yend, const xlColor &color, bool wrap) {
    if (ystart > yend) {
        int i = ystart;
        ystart = yend;
        yend = i;
    }
    for (int y = ystart; y <= yend; y++) {
        SetPixel(x, y, color, wrap);
    }
}
void RenderBuffer::DrawBox(int x1, int y1, int x2, int y2, const xlColor& color, bool wrap) {
    if (y1 > y2) {
        int i = y1;
        y1 = y2;
        y2 = i;
    }
    if (x1 > x2) {
        int i = x1;
        x1 = x2;
        x2 = i;
    }
    for (int x = x1; x <= x2; x++) {
        for (int y = y1; y <= y2; y++) {
            SetPixel(x, y, color, wrap);
        }
    }
}

// Bresenham's line algorithm
void RenderBuffer::DrawLine( const int x0_, const int y0_, const int x1_, const int y1_, const xlColor& color, bool useAlpha)
{
    int x0 = x0_;
    int x1 = x1_;
    int y0 = y0_;
    int y1 = y1_;

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2;

  for(;;){
    SetPixel(x0,y0, color, false, useAlpha);
    if (x0==x1 && y0==y1) break;
    int e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void RenderBuffer::DrawThickLine(const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color, int thickness, bool useAlpha)
{
    if (thickness < 1) return;
    if (thickness == 1)
    {
        DrawLine(x1_, y1_, x2_, y2_, color, useAlpha);
    }
    else
    {
        DrawCircle(x1_, y1_, thickness / 2, color, true);
        DrawCircle(x2_, y2_, thickness / 2, color, true);
        for (int i =0; i < thickness; i++)
        {
            int adjust = i - thickness / 2;
            DrawLine(x1_ + adjust, y1_, x2_ + adjust, y2_, color, useAlpha);
            DrawLine(x1_, y1_ + adjust, x2_, y2_ + adjust, color, useAlpha);
            DrawLine(x1_ + adjust, y1_ + adjust, x2_ + adjust, y2_ + adjust, color, useAlpha);
        }
    }
}

void RenderBuffer::DrawThickLine( const int x0_, const int y0_, const int x1_, const int y1_, const xlColor& color, bool direction )
{
    int x0 = x0_;
    int x1 = x1_;
    int y0 = y0_;
    int y1 = y1_;
    int lastx = x0;
    int lasty = y0;

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    SetPixel(x0,y0, color);
    if( (x0 != lastx) && (y0 != lasty) && (x0_ != x1_) && (y0_ != y1_) )
    {
        int fix = 0;
        if( x0 > lastx ) fix += 1;
        if( y0 > lasty ) fix += 2;
        if( direction  ) fix += 4;
        switch (fix)
        {
        case 2:
        case 4:
            if( x0 < BufferWi -2 ) SetPixel(x0+1,y0, color);
            break;
        case 3:
        case 5:
            if( x0 > 0 ) SetPixel(x0-1,y0, color);
            break;
        case 0:
        case 1:
            if( y0 < BufferHt -2 )SetPixel(x0,y0+1, color);
            break;
        case 6:
        case 7:
            if( y0 > 0 )SetPixel(x0,y0-1, color);
            break;
        default: break;
        }
    }
    lastx = x0;
    lasty = y0;
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void RenderBuffer::DrawFadingCircle(int x0, int y0, int radius, const xlColor& rgb, bool wrap)
{
    HSVValue hsv(rgb);
    xlColor color(rgb);

    double full_brightness = hsv.value;

    for (int x = -radius; x < radius; ++x) {
        for (int y = -radius; y < radius; ++y) {
            double d = std::sqrt(x * x + y * y);
            if (d <= radius) {
                if (allowAlpha) {
                    double alpha = (double)rgb.alpha - ((double)rgb.alpha * d) / double(radius);
                    if (alpha > 0.0) {
                        color.alpha = alpha;
                        SetPixel(x + x0, y + y0, color, wrap, false);
                    }
                }
                else {
                    double alpha = full_brightness - (full_brightness * d) / (double)radius;
                    if (alpha > 0.0) {
                        hsv.value = alpha;
                        color = hsv;
                        SetPixel(x + x0, y + y0, color, wrap);
                    }
                }
            }
        }
    }
}

void RenderBuffer::DrawCircle(int x0, int y0, int radius, const xlColor& rgb, bool filled, bool wrap)
{
    int x = radius;
    int y = 0;
    int radiusError = 1 - x;

    while(x >= y) {
        if (!filled) {
            SetPixel(x + x0, y + y0, rgb, wrap);
            SetPixel(y + x0, x + y0, rgb, wrap);
            SetPixel(-x + x0, y + y0, rgb, wrap);
            SetPixel(-y + x0, x + y0, rgb, wrap);
            SetPixel(-x + x0, -y + y0, rgb, wrap);
            SetPixel(-y + x0, -x + y0, rgb, wrap);
            SetPixel(x + x0, -y + y0, rgb, wrap);
            SetPixel(y + x0, -x + y0, rgb, wrap);
        } else {
            DrawVLine(x0 - x, y0 - y, y0 + y, rgb, wrap);
            DrawVLine(x0 + x, y0 - y, y0 + y, rgb, wrap);
            DrawVLine(x0 - y, y0 - x, y0 + x, rgb, wrap);
            DrawVLine(x0 + y, y0 - x, y0 + x, rgb, wrap);
        }
        y++;
        if (radiusError<0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x) + 1;
        }
    }
}

void RenderBuffer::Fill(const xlColor& color) {
    std::fill_n(pixels, pixelVector.size(), color);
}


// 0,0 is lower left
void RenderBuffer::GetPixel(int x, int y, xlColor &color) const
{
    // I also dont like this ... I shouldnt need to check against pixel size
    int pidx = y * BufferWi + x;
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && pidx < pixelVector.size()) {
        color = pixels[pidx];
    } else {
        color = xlBLACK;
    }
}

const xlColor& RenderBuffer::GetPixel(int x, int y) const {
    int pidx = y * BufferWi + x;
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && pidx < pixelVector.size()) {
        return pixels[pidx];
    }
    return xlBLACK;
}

// 0,0 is lower left
void RenderBuffer::SetTempPixel(int x, int y, const xlColor& color) {
    int pidx = y * BufferWi + x;
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && pidx < tempbufVector.size()) {
        tempbuf[pidx] = color;
    }
}

void RenderBuffer::SetTempPixel(int x, int y, const xlColor & color, int alpha) {

    xlColor c(color.Red(), color.Green(), color.Blue(), alpha);
    SetTempPixel(x, y, c);
}

// 0,0 is lower left
void RenderBuffer::GetTempPixel(int x, int y, xlColor& color)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && y * BufferWi + x < tempbufVector.size()) {
        color = tempbuf[y * BufferWi + x];
    }
}

const xlColor& RenderBuffer::GetTempPixel(int x, int y) {
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && y * BufferWi + x < tempbufVector.size()) {
        return tempbuf[y * BufferWi + x];
    }
    return xlBLACK;
}

const xlColor& RenderBuffer::GetTempPixelRGB(int x, int y)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && y * BufferWi + x < tempbufVector.size()) {
        return tempbuf[y * BufferWi + x];
    }
    return xlBLACK;
}

void RenderBuffer::SetState(int period, bool ResetState, const std::string& model_name)
{
    if (ResetState) {
        needToInit = true;
    }
    curPeriod = period;
    cur_model = model_name;
    curPeriod = period;
    palette.UpdateForProgress(GetEffectTimeIntervalPosition());
    dmx_buffer = false;
    Model* m = GetModel();
    if (m != nullptr) {
        if (m->GetDisplayAs().rfind("Dmx", 0) == 0) {
            dmx_buffer = true;
        }
    }
}

void RenderBuffer::ClearTempBuf()
{
    for (size_t i = 0; i < tempbufVector.size(); i++) {
        tempbuf[i].Set(0, 0, 0, 0);
    }
}
void RenderBuffer::CopyTempBufToPixels() {
    memcpy(pixels, tempbuf, pixelVector.size() * 4);
}
void RenderBuffer::CopyPixelsToTempBuf() {
    memcpy(tempbuf, pixels, pixelVector.size() * 4);
}

float RenderBuffer::GetEffectTimeIntervalPosition(float cycles) const {
    if (curEffEndPer == curEffStartPer) {
        return 0.0f;
    }
    float periods = curEffEndPer - curEffStartPer + 1; //inclusive
    float periodsPerCycle = periods / cycles;
    if (periodsPerCycle <= 1.0) {
        return 0.0f;
    }
    float retval = (float)(curPeriod - curEffStartPer);
    while (retval >= periodsPerCycle) {
        retval -= periodsPerCycle;
    }
    retval /= (periodsPerCycle - 1);
    return retval > 1.0f ? 1.0f : retval;
}

float RenderBuffer::GetEffectTimeIntervalPosition() const
{
    if (curEffEndPer == curEffStartPer) {
        return 0.0;
    }
    return (float)(curPeriod - curEffStartPer) / (float)(curEffEndPer - curEffStartPer);
}

void RenderBuffer::SetEffectDuration(int startMsec, int endMsec)
{
    curEffStartPer = startMsec / frameTimeInMs;
    curEffEndPer = (endMsec - 1) / frameTimeInMs;
}

void RenderBuffer::GetEffectPeriods(int& start, int& endp) const {
    start = curEffStartPer;
    endp = curEffEndPer;
}

void RenderBuffer::SetDisplayListHRect(Effect *eff, int idx, float x1, float y1, float x2, float y2,
                                     const xlColor &c1, const xlColor &c2) {
    SetDisplayListRect(eff, idx, x1, y1, x2, y2, c1, c1, c2, c2);
}
void RenderBuffer::SetDisplayListVRect(Effect *eff, int idx, float x1, float y1, float x2, float y2,
                                     const xlColor &c1, const xlColor &c2) {
    SetDisplayListRect(eff, idx, x1, y1, x2, y2, c1, c2, c1, c2);
}
void RenderBuffer::SetDisplayListRect(Effect *eff, int idx, float x1, float y1, float x2, float y2,
                                    const xlColor &cx1y1, const xlColor &cx1y2,
                                    const xlColor &cx2y1, const xlColor &cx2y2) {

    xlColor* colorMask = eff->GetColorMask();
    xlColor maskcx1y1 = cx1y1;
    xlColor maskcx1y2 = cx1y2;
    xlColor maskcx2y1 = cx2y1;
    xlColor maskcx2y2 = cx2y2;
    maskcx1y1.ApplyMask(colorMask);
    maskcx1y2.ApplyMask(colorMask);
    maskcx2y1.ApplyMask(colorMask);
    maskcx2y2.ApplyMask(colorMask);

    eff->GetBackgroundDisplayList()[idx].color = maskcx1y1;
    eff->GetBackgroundDisplayList()[idx+1].color = maskcx1y2;
    eff->GetBackgroundDisplayList()[idx+2].color = maskcx2y2;
    eff->GetBackgroundDisplayList()[idx+3].color = maskcx2y2;
    eff->GetBackgroundDisplayList()[idx+4].color = maskcx2y1;
    eff->GetBackgroundDisplayList()[idx+5].color = maskcx1y1;

    eff->GetBackgroundDisplayList()[idx].x = x1;
    eff->GetBackgroundDisplayList()[idx+1].x = x1;
    eff->GetBackgroundDisplayList()[idx+2].x = x2;
    eff->GetBackgroundDisplayList()[idx+3].x = x2;
    eff->GetBackgroundDisplayList()[idx+4].x = x2;
    eff->GetBackgroundDisplayList()[idx+5].x = x1;

    eff->GetBackgroundDisplayList()[idx].y = y1;
    eff->GetBackgroundDisplayList()[idx+1].y = y2;
    eff->GetBackgroundDisplayList()[idx+2].y = y2;
    eff->GetBackgroundDisplayList()[idx+3].y = y2;
    eff->GetBackgroundDisplayList()[idx+4].y = y1;
    eff->GetBackgroundDisplayList()[idx+5].y = y1;
}

void RenderBuffer::CopyPixelsToDisplayListX(Effect *eff, int row, int sx, int ex, int inc) {
    std::lock_guard<std::recursive_mutex> lock(eff->GetBackgroundDisplayList().lock);
    int count = curEffEndPer - curEffStartPer + 1;

    int total = curEffEndPer - curEffStartPer + 1;
    float x = float(curPeriod - curEffStartPer) / float(total);
    float x2 = (curPeriod - curEffStartPer + 1.0) / float(total);
    xlColor c;

    int cur = 0;
    for (int p = sx; p <= ex; p += inc) {
        float y = float(p - sx) / float(ex - sx + 1.0);
        float y2 = float(p - sx + 1.0) / float(ex - sx + 1.0);
        GetPixel(p, row, c);

        int idx = cur * count + (curPeriod - curEffStartPer);
        cur++;
        SetDisplayListHRect(eff, idx*6, x, y, x2, y2, c, c);
    }
}

double RenderBuffer::calcAccel(double ratio, double accel)
{
    if( accel == 0 ) return ratio;

    double pct_accel = (std::abs(accel) - 1.0) / 9.0;
    double new_accel1 = pct_accel * 5 + (1.0 - pct_accel) * 1.5;
    double new_accel2 = 1.5 + (ratio * new_accel1);
    double final_accel = pct_accel * new_accel2 + (1.0 - pct_accel) * new_accel1;

    if( accel > 0 )
    {
        return std::pow(ratio, final_accel);
    }
    else
    {
        return (1.0 - std::pow(1.0 - ratio, new_accel1));
    }
}

// create a copy of the buffer suitable only for copying out pixel data and fake rendering
RenderBuffer::RenderBuffer(RenderBuffer& buffer) : pixelVector(buffer.pixels, &buffer.pixels[buffer.pixelVector.size()])
{
    _isCopy = true;
    frame = buffer.frame;
    curPeriod = buffer.curPeriod;
    curEffStartPer = buffer.curEffStartPer;
    curEffEndPer = buffer.curEffEndPer;
    frameTimeInMs = buffer.frameTimeInMs;
    isTransformed = buffer.isTransformed;
    fadeinsteps = buffer.fadeinsteps;
    fadeoutsteps = buffer.fadeoutsteps;
    needToInit = buffer.needToInit;
    tempInt = buffer.tempInt;
    tempInt2 = buffer.tempInt2;
    allowAlpha = buffer.allowAlpha;
    dmx_buffer = buffer.dmx_buffer;
    _nodeBuffer = buffer._nodeBuffer;
    BufferHt = buffer.BufferHt;
    BufferWi = buffer.BufferWi;
    ModelBufferHt = buffer.ModelBufferHt;
    ModelBufferWi = buffer.ModelBufferWi;
    infoCache = buffer.infoCache;

    pixels = &pixelVector[0];
    _textDrawingContext = buffer._textDrawingContext;
    _pathDrawingContext = buffer._pathDrawingContext;
    gpuRenderData = nullptr;
}

void RenderBuffer::Forget()
{
    // Forget some stuff as this is a fake render buffer and we dont want it destroyed
    infoCache.clear();
    _textDrawingContext = nullptr;
    _pathDrawingContext = nullptr;
}

void RenderBuffer::SetPixelDMXModel(int x, int y, const xlColor& color)
{
    Model* model_info = GetModel();
    if (model_info != nullptr) {
        if (x != 0 || y != 0) return;  //Only render colors for the first pixel

        if (pixelVector.size() == 1) { //pixel size equals 1 when putting "on" effect at node level
            pixels[0] = color;
            return;
        }
        xlColor c;
        DmxModel* dmx = (DmxModel*)model_info;
        if (dmx->HasColorAbility()) {
            DmxColorAbility* dmx_color = dmx->GetColorAbility();

            int white_channel = dmx_color->GetWhiteChannel();
            if (white_channel > 0 && color.red == color.green && color.red == color.blue)
            {
                c.red = color.red;
                c.green = color.red;
                c.blue = color.red;
                if (pixelVector.size() > white_channel - 1) pixels[white_channel - 1] = c;
            }
            else
            {
                int red_channel = dmx_color->GetRedChannel();
                int grn_channel = dmx_color->GetGreenChannel();
                int blu_channel = dmx_color->GetBlueChannel();
                if (red_channel != 0) {
                    c.red = color.red;
                    c.green = color.red;
                    c.blue = color.red;
                    if (pixelVector.size() > red_channel - 1) pixels[red_channel - 1] = c;
                }
                if (grn_channel != 0) {
                    c.red = color.green;
                    c.green = color.green;
                    c.blue = color.green;
                    if (pixelVector.size() > grn_channel - 1) pixels[grn_channel - 1] = c;
                }
                if (blu_channel != 0) {
                    c.red = color.blue;
                    c.green = color.blue;
                    c.blue = color.blue;
                    if (pixelVector.size() > blu_channel - 1) pixels[blu_channel - 1] = c;
                }
            }
        }
    }
}
