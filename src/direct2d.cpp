// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

/**
    Coordinate System Strategy:

    Direct2D uses a top-left origin coordinate system by default, which matches
    the natural LUI coordinate system. Unlike CoreGraphics/Cairo which require
    flipping transformations, Direct2D paths, fills, strokes, and rectangles
    work directly with top-left coordinates.

    Text rendering in DirectWrite also uses top-left origin naturally, so no
    coordinate flipping is needed for text operations.
*/

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <windows.h>

// Link required libraries
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#include "pugl/pugl.h"

#include <lui/direct2d.hpp>
#include <lui/graphics.hpp>
#include <lui/widget.hpp>

// Forward declare the pugl surface structure
typedef struct {
    ID2D1Factory* d2dFactory;
    ID2D1HwndRenderTarget* renderTarget;
    IDWriteFactory* writeFactory;
} PuglWinDirect2DSurface;

extern "C" {
const PuglBackend* puglDirect2DBackend();
}

// Helper function to release COM interfaces safely
namespace {
template <class Interface>
void SafeRelease (Interface** ppInterfaceToRelease) {
    if (*ppInterfaceToRelease != nullptr) {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = nullptr;
    }
}
}

namespace lui {
namespace d2d {

class Context : public DrawingContext {
public:
    explicit Context() {
        stack.reserve (64);
    }

    ~Context() {
        release_resources();
    }

    bool begin_frame (ID2D1RenderTarget* renderTarget, IDWriteFactory* dwFactory, lui::Bounds bounds) {
        rt           = renderTarget;
        writeFactory = dwFactory;
        state        = {};
        state.font   = Font (14.0f);
        stack.clear();
        release_path_resources();

        if (! rt)
            return false;

        // Clear to white background
        rt->Clear (D2D1::ColorF (D2D1::ColorF::White));

        // Initialize path
        create_new_path();

        // Set initial clip without tracking (will be cleaned up in end_frame)
        rt->PushAxisAlignedClip (
            D2D1::RectF (
                static_cast<float> (bounds.x),
                static_cast<float> (bounds.y),
                static_cast<float> (bounds.x + bounds.width),
                static_cast<float> (bounds.y + bounds.height)),
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        state.clip = bounds.as<double>();
        state.clip_depth = 1;
        return true;
    }

    void end_frame() {
        // Pop any remaining clips
        while (state.clip_depth > 0) {
            rt->PopAxisAlignedClip();
            state.clip_depth--;
        }
        release_path_resources();
        rt           = nullptr;
        writeFactory = nullptr;
    }

    double device_scale() const noexcept override {
        if (! rt)
            return 1.0;

        FLOAT dpiX, dpiY;
        rt->GetDpi (&dpiX, &dpiY);
        return static_cast<double> (dpiX / 96.0);
    }

    void save() override {
        stack.push_back (state);
    }

    void restore() override {
        if (stack.empty())
            return;
        
        // Pop any extra clips that were pushed since save
        while (state.clip_depth > stack.back().clip_depth) {
            rt->PopAxisAlignedClip();
            state.clip_depth--;
        }
        
        std::swap (state, stack.back());
        stack.pop_back();
    }

    void set_line_width (double width) override {
        state.line_width = static_cast<float> (width);
    }

    void clear_path() override {
        release_path_resources();
        create_new_path();
        current_pos = D2D1::Point2F (0, 0);
    }

    void move_to (double x1, double y1) override {
        if (! geometrySink)
            return;

        if (figure_active) {
            geometrySink->EndFigure (D2D1_FIGURE_END_OPEN);
            figure_active = false;
        }

        current_pos = D2D1::Point2F (static_cast<float> (x1), static_cast<float> (y1));
        geometrySink->BeginFigure (current_pos, D2D1_FIGURE_BEGIN_FILLED);
        figure_active = true;
    }

    void line_to (double x1, double y1) override {
        if (! geometrySink || ! figure_active)
            return;

        D2D1_POINT_2F pt = D2D1::Point2F (static_cast<float> (x1), static_cast<float> (y1));
        geometrySink->AddLine (pt);
        current_pos = pt;
    }

    void quad_to (double x1, double y1, double x2, double y2) override {
        // Convert quadratic bezier to cubic
        double cx1 = current_pos.x + 2.0 / 3.0 * (x1 - current_pos.x);
        double cy1 = current_pos.y + 2.0 / 3.0 * (y1 - current_pos.y);
        double cx2 = x2 + 2.0 / 3.0 * (x1 - x2);
        double cy2 = y2 + 2.0 / 3.0 * (y1 - y2);

        cubic_to (cx1, cy1, cx2, cy2, x2, y2);
    }

    void cubic_to (double x1, double y1, double x2, double y2, double x3, double y3) override {
        if (! geometrySink || ! figure_active)
            return;

        D2D1_BEZIER_SEGMENT bezier;
        bezier.point1 = D2D1::Point2F (static_cast<float> (x1), static_cast<float> (y1));
        bezier.point2 = D2D1::Point2F (static_cast<float> (x2), static_cast<float> (y2));
        bezier.point3 = D2D1::Point2F (static_cast<float> (x3), static_cast<float> (y3));
        geometrySink->AddBezier (bezier);
        current_pos = bezier.point3;
    }

    void close_path() override {
        if (! geometrySink || ! figure_active)
            return;

        geometrySink->EndFigure (D2D1_FIGURE_END_CLOSED);
        figure_active = false;
    }

    void fill() override {
        if (! rt || ! pathGeometry)
            return;

        finalize_path();

        auto c = state.color;
        ID2D1SolidColorBrush* brush = nullptr;
        HRESULT hr                  = rt->CreateSolidColorBrush (
            D2D1::ColorF (c.red() / 255.0f, c.green() / 255.0f, c.blue() / 255.0f, c.alpha() / 255.0f),
            &brush);

        if (SUCCEEDED (hr)) {
            rt->FillGeometry (pathGeometry, brush);
            SafeRelease (&brush);
        }
    }

    void stroke() override {
        if (! rt || ! pathGeometry)
            return;

        finalize_path();

        auto c = state.color;
        ID2D1SolidColorBrush* brush = nullptr;
        HRESULT hr                  = rt->CreateSolidColorBrush (
            D2D1::ColorF (c.red() / 255.0f, c.green() / 255.0f, c.blue() / 255.0f, c.alpha() / 255.0f),
            &brush);

        if (SUCCEEDED (hr)) {
            ID2D1StrokeStyle* strokeStyle = nullptr;
            // Create stroke style with round caps
            ID2D1Factory* factory = nullptr;
            rt->GetFactory (&factory);
            if (factory) {
                D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties (
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_CAP_STYLE_ROUND,
                    D2D1_LINE_JOIN_ROUND);

                factory->CreateStrokeStyle (strokeProps, nullptr, 0, &strokeStyle);
                factory->Release();
            }

            rt->DrawGeometry (pathGeometry, brush, state.line_width, strokeStyle);
            SafeRelease (&brush);
            SafeRelease (&strokeStyle);
        }
    }

    void translate (double x, double y) override {
        if (! rt)
            return;

        D2D1_MATRIX_3X2_F transform;
        rt->GetTransform (&transform);

        D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation (
            static_cast<float> (x),
            static_cast<float> (y));

        rt->SetTransform (transform * translation);

        state.clip.x -= x;
        state.clip.y -= y;
    }

    void transform (const Transform& mat) override {
        if (! rt)
            return;

        D2D1_MATRIX_3X2_F transform;
        rt->GetTransform (&transform);

        D2D1_MATRIX_3X2_F newTransform = D2D1::Matrix3x2F (
            static_cast<float> (mat.m00),
            static_cast<float> (mat.m10),
            static_cast<float> (mat.m01),
            static_cast<float> (mat.m11),
            static_cast<float> (mat.m02),
            static_cast<float> (mat.m12));

        rt->SetTransform (transform * newTransform);
    }

    void clip (const Rectangle<int>& r) override {
        state.clip = r.as<double>();
        if (! rt)
            return;

        // Push axis-aligned clip
        rt->PushAxisAlignedClip (
            D2D1::RectF (
                static_cast<float> (r.x),
                static_cast<float> (r.y),
                static_cast<float> (r.x + r.width),
                static_cast<float> (r.y + r.height)),
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        state.clip_depth++;
    }

    void exclude_clip (const Rectangle<int>&) override {
        // Direct2D doesn't have direct exclude clip support
        // This would require creating geometry and using PushLayer
        // For now, we skip implementation as it's marked TODO in GDI too
    }

    Rectangle<int> last_clip() const override {
        return state.clip.as<int>();
    }

    Font font() const noexcept override {
        return state.font;
    }

    void set_font (const Font& font) override {
        state.font = font;
    }

    void set_fill (const Fill& fill) override {
        if (fill.is_color()) {
            state.color = fill.color();
        }
    }

    void fill_rect (const Rectangle<double>& r) override {
        if (! rt)
            return;

        auto c = state.color;
        ID2D1SolidColorBrush* brush = nullptr;
        HRESULT hr                  = rt->CreateSolidColorBrush (
            D2D1::ColorF (c.red() / 255.0f, c.green() / 255.0f, c.blue() / 255.0f, c.alpha() / 255.0f),
            &brush);

        if (SUCCEEDED (hr)) {
            rt->FillRectangle (
                D2D1::RectF (
                    static_cast<float> (r.x),
                    static_cast<float> (r.y),
                    static_cast<float> (r.x + r.width),
                    static_cast<float> (r.y + r.height)),
                brush);
            SafeRelease (&brush);
        }
    }

    FontMetrics font_metrics() const noexcept override {
        if (! writeFactory)
            return {};

        IDWriteTextFormat* textFormat = nullptr;
        HRESULT hr                    = writeFactory->CreateTextFormat (
            L"Roboto",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            state.font.height(),
            L"en-us",
            &textFormat);

        if (FAILED (hr))
            return {};

        IDWriteFontCollection* fontCollection = nullptr;
        textFormat->GetFontCollection (&fontCollection);

        UINT32 familyIndex = 0;
        BOOL exists        = FALSE;
        if (fontCollection) {
            fontCollection->FindFamilyName (L"Roboto", &familyIndex, &exists);
        }

        IDWriteFontFamily* fontFamily = nullptr;
        if (exists && fontCollection) {
            fontCollection->GetFontFamily (familyIndex, &fontFamily);
        }

        IDWriteFont* font = nullptr;
        if (fontFamily) {
            fontFamily->GetFirstMatchingFont (
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                &font);
        }

        DWRITE_FONT_METRICS fontMetrics;
        if (font) {
            font->GetMetrics (&fontMetrics);
        }

        SafeRelease (&font);
        SafeRelease (&fontFamily);
        SafeRelease (&fontCollection);
        SafeRelease (&textFormat);

        if (! font) {
            // Return approximate values if we couldn't get font
            const float height = state.font.height();
            return {
                height * 0.8,
                height * 0.2,
                height,
                height * 0.5,
                height
            };
        }

        const float fontSize   = state.font.height();
        const float designSize = static_cast<float> (fontMetrics.designUnitsPerEm);
        const float ascent     = fontSize * fontMetrics.ascent / designSize;
        const float descent    = fontSize * fontMetrics.descent / designSize;
        const float height     = ascent + descent;

        return {
            static_cast<double> (ascent),
            static_cast<double> (descent),
            static_cast<double> (height),
            static_cast<double> (fontSize / 2.0),
            static_cast<double> (height)
        };
    }

    TextMetrics text_metrics (std::string_view text) const noexcept override {
        if (! writeFactory || ! rt)
            return {};

        std::wstring wtext (text.begin(), text.end());

        IDWriteTextFormat* textFormat = nullptr;
        HRESULT hr                    = writeFactory->CreateTextFormat (
            L"Roboto",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            state.font.height(),
            L"en-us",
            &textFormat);

        if (FAILED (hr))
            return {};

        IDWriteTextLayout* textLayout = nullptr;
        hr                            = writeFactory->CreateTextLayout (
            wtext.c_str(),
            static_cast<UINT32> (wtext.length()),
            textFormat,
            10000.0f,
            10000.0f,
            &textLayout);

        SafeRelease (&textFormat);

        if (FAILED (hr))
            return {};

        DWRITE_TEXT_METRICS metrics;
        textLayout->GetMetrics (&metrics);
        SafeRelease (&textLayout);

        return {
            static_cast<double> (metrics.width),
            static_cast<double> (metrics.height),
            0.0,
            0.0,
            static_cast<double> (metrics.width),
            static_cast<double> (metrics.height)
        };
    }

    bool show_text (std::string_view text) override {
        if (! rt || ! writeFactory)
            return false;

        std::wstring wtext (text.begin(), text.end());

        IDWriteTextFormat* textFormat = nullptr;
        HRESULT hr                    = writeFactory->CreateTextFormat (
            L"Roboto",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            state.font.height(),
            L"en-us",
            &textFormat);

        if (FAILED (hr))
            return false;

        // Get font metrics to calculate baseline adjustment
        auto metrics = font_metrics();

        auto c = state.color;
        ID2D1SolidColorBrush* brush = nullptr;
        hr                          = rt->CreateSolidColorBrush (
            D2D1::ColorF (c.red() / 255.0f, c.green() / 255.0f, c.blue() / 255.0f, c.alpha() / 255.0f),
            &brush);

        if (SUCCEEDED (hr)) {
            // DirectWrite uses top-left positioning, current_pos represents baseline
            // Adjust Y coordinate by subtracting ascent
            D2D1_RECT_F layoutRect = D2D1::RectF (
                current_pos.x,
                current_pos.y - static_cast<float> (metrics.ascent),
                10000.0f,
                10000.0f);

            rt->DrawText (
                wtext.c_str(),
                static_cast<UINT32> (wtext.length()),
                textFormat,
                layoutRect,
                brush);

            SafeRelease (&brush);
        }

        SafeRelease (&textFormat);
        return SUCCEEDED (hr);
    }

    void draw_image (Image i, Transform matrix) override {
        if (! rt)
            return;

        // Create a D2D bitmap from the image data
        D2D1_BITMAP_PROPERTIES bitmapProps;
        bitmapProps.pixelFormat.format    = DXGI_FORMAT_B8G8R8A8_UNORM;
        bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
        bitmapProps.dpiX                  = 96.0f;
        bitmapProps.dpiY                  = 96.0f;

        ID2D1Bitmap* bitmap = nullptr;
        HRESULT hr          = rt->CreateBitmap (
            D2D1::SizeU (i.width(), i.height()),
            i.data(),
            i.stride(),
            bitmapProps,
            &bitmap);

        if (SUCCEEDED (hr)) {
            // Save current transform
            D2D1_MATRIX_3X2_F oldTransform;
            rt->GetTransform (&oldTransform);

            // Apply the transformation matrix
            D2D1_MATRIX_3X2_F newTransform = D2D1::Matrix3x2F (
                static_cast<float> (matrix.m00),
                static_cast<float> (matrix.m10),
                static_cast<float> (matrix.m01),
                static_cast<float> (matrix.m11),
                static_cast<float> (matrix.m02),
                static_cast<float> (matrix.m12));

            rt->SetTransform (oldTransform * newTransform);

            // Draw the bitmap
            D2D1_RECT_F destRect = D2D1::RectF (
                0.0f,
                0.0f,
                static_cast<float> (i.width()),
                static_cast<float> (i.height()));

            rt->DrawBitmap (
                bitmap,
                destRect,
                1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                nullptr);

            // Restore transform
            rt->SetTransform (oldTransform);

            SafeRelease (&bitmap);
        }
    }

private:
    void create_new_path() {
        if (! rt)
            return;

        ID2D1Factory* factory = nullptr;
        rt->GetFactory (&factory);

        if (factory) {
            factory->CreatePathGeometry (&pathGeometry);
            if (pathGeometry) {
                pathGeometry->Open (&geometrySink);
            }
            factory->Release();
        }

        figure_active = false;
    }

    void finalize_path() {
        if (geometrySink && figure_active) {
            geometrySink->EndFigure (D2D1_FIGURE_END_OPEN);
            figure_active = false;
        }

        if (geometrySink) {
            geometrySink->Close();
            SafeRelease (&geometrySink);
        }
    }

    void release_path_resources() {
        if (geometrySink && figure_active) {
            geometrySink->EndFigure (D2D1_FIGURE_END_OPEN);
            figure_active = false;
        }

        SafeRelease (&geometrySink);
        SafeRelease (&pathGeometry);
    }

    void release_resources() {
        release_path_resources();
        rt           = nullptr;
        writeFactory = nullptr;
    }

    struct State {
        Font font;
        Color color;
        Rectangle<double> clip;
        float line_width { 1.0f };
        int clip_depth { 0 };
    };

    ID2D1RenderTarget* rt { nullptr };
    IDWriteFactory* writeFactory { nullptr };
    ID2D1PathGeometry* pathGeometry { nullptr };
    ID2D1GeometrySink* geometrySink { nullptr };
    D2D1_POINT_2F current_pos { 0, 0 };
    bool figure_active { false };

    State state;
    std::vector<State> stack;
};

class View : public lui::View {
public:
    View (Main& m, Widget& w)
        : lui::View (m, w) {
        set_backend ((uintptr_t) puglDirect2DBackend());
        set_view_hint (PUGL_DOUBLE_BUFFER, PUGL_FALSE);
        set_view_hint (PUGL_RESIZABLE, PUGL_TRUE);
        puglSetViewString ((PuglView*) c_obj(), PUGL_WINDOW_TITLE, w.name().c_str());
    }

    ~View() {}

    void expose (Bounds frame) override {
        auto surface = (PuglWinDirect2DSurface*) puglGetContext (_view);
        if (! surface || ! surface->renderTarget)
            return;

        auto rt = surface->renderTarget;
        auto writeFactory = surface->writeFactory;

        const auto scale = scale_factor();
        if (scale != 1.0 && scale != _last_scale) {
            rt->SetDpi (static_cast<float> (scale * 96.0), static_cast<float> (scale * 96.0));
            _last_scale  = scale;
        }

        if (_context->begin_frame (rt, writeFactory, frame)) {
            render (*_context);
            _context->end_frame();
        }
    }

    void created() override {
        _context = std::make_unique<Context>();
        _view    = (PuglView*) c_obj();
        assert (_view != nullptr && _context != nullptr);
    }

    void destroyed() override {
        _view = nullptr;
        _context.reset();
    }

private:
    using Parent = lui::View;
    PuglView* _view { nullptr };
    std::unique_ptr<Context> _context;
    double _last_scale { 1.0 };
};

} // namespace d2d

std::unique_ptr<lui::View> Direct2D::create_view (Main& c, Widget& w) {
    return std::make_unique<d2d::View> (c, w);
}

} // namespace lui
