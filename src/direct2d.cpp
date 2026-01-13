// Copyright 2026 Michael Fisher <mfisher@lvtk.org>
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
#include <iostream>
#include <vector>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <windows.h>

// Link required libraries
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#include <lui/direct2d.hpp>
#include <lui/graphics.hpp>
#include <lui/widget.hpp>

#include "pugl/src/stub.h"

extern "C" {
const PuglBackend* puglDirect2DBackend();
}

#define LUI_D2D_DEFAULT_FONT L"Segoe UI"

namespace lui {
namespace d2d {

class Context : public DrawingContext {
public:
    explicit Context (ID2D1RenderTarget* target = nullptr)
        : rt (target) {
        stack.reserve (64);
    }

    ~Context() {
        release_resources();
    }

    bool begin_frame (ID2D1RenderTarget* _rt, lui::Bounds bounds) {
        rt    = _rt;
        state = {};
        stack.clear();
        release_resources();

        if (rt) {
            // Get DWrite factory from render target
            ID2D1Factory* factory = nullptr;
            rt->GetFactory (&factory);
            if (factory) {
                factory->Release();
            }
        }

        this->clip (bounds);
        return true;
    }

    void end_frame() {
        release_resources();
        rt = nullptr;
    }

    double device_scale() const noexcept override {
        assert (rt != nullptr);
        FLOAT dpiX, dpiY;
        rt->GetDpi (&dpiX, &dpiY);
        return static_cast<double> (dpiX / 96.0); // 96 DPI is default
    }

    void save() override {
        stack.push_back (state);
        if (current_layer) {
            rt->PushLayer (D2D1::LayerParameters(), current_layer);
        }
    }

    void restore() override {
        if (current_layer) {
            rt->PopLayer();
        }
        if (stack.empty())
            return;
        std::swap (state, stack.back());
        stack.pop_back();
    }

    void set_line_width (double width) override {
        state.line_width = static_cast<FLOAT> (width);
    }

    void clear_path() override {
        if (geometry_sink) {
            geometry_sink->Close();
            geometry_sink->Release();
            geometry_sink = nullptr;
        }
        if (path_geometry) {
            path_geometry->Release();
            path_geometry = nullptr;
        }

        // Create new path geometry
        ID2D1Factory* factory = nullptr;
        rt->GetFactory (&factory);
        if (factory) {
            factory->CreatePathGeometry (&path_geometry);
            if (path_geometry) {
                path_geometry->Open (&geometry_sink);
                needs_begin_figure = true;
            }
            factory->Release();
        }
    }

    void move_to (double x1, double y1) override {
        ensure_path();
        if (geometry_sink) {
            if (! needs_begin_figure) {
                geometry_sink->EndFigure (D2D1_FIGURE_END_OPEN);
            }
            geometry_sink->BeginFigure (
                D2D1::Point2F (static_cast<FLOAT> (x1), static_cast<FLOAT> (y1)),
                D2D1_FIGURE_BEGIN_FILLED);
            needs_begin_figure = false;
            current_point      = D2D1::Point2F (static_cast<FLOAT> (x1), static_cast<FLOAT> (y1));
        }
    }

    void line_to (double x1, double y1) override {
        ensure_path();
        ensure_figure();
        if (geometry_sink) {
            current_point = D2D1::Point2F (static_cast<FLOAT> (x1), static_cast<FLOAT> (y1));
            geometry_sink->AddLine (current_point);
        }
    }

    void quad_to (double x1, double y1, double x2, double y2) override {
        ensure_path();
        ensure_figure();
        if (geometry_sink) {
            D2D1_QUADRATIC_BEZIER_SEGMENT segment = {
                D2D1::Point2F (static_cast<FLOAT> (x1), static_cast<FLOAT> (y1)),
                D2D1::Point2F (static_cast<FLOAT> (x2), static_cast<FLOAT> (y2))
            };
            geometry_sink->AddQuadraticBezier (segment);
            current_point = segment.point2;
        }
    }

    void cubic_to (double x1, double y1, double x2, double y2, double x3, double y3) override {
        ensure_path();
        ensure_figure();
        if (geometry_sink) {
            D2D1_BEZIER_SEGMENT segment = {
                D2D1::Point2F (static_cast<FLOAT> (x1), static_cast<FLOAT> (y1)),
                D2D1::Point2F (static_cast<FLOAT> (x2), static_cast<FLOAT> (y2)),
                D2D1::Point2F (static_cast<FLOAT> (x3), static_cast<FLOAT> (y3))
            };
            geometry_sink->AddBezier (segment);
            current_point = segment.point3;
        }
    }

    void close_path() override {
        if (geometry_sink && ! needs_begin_figure) {
            geometry_sink->EndFigure (D2D1_FIGURE_END_CLOSED);
            needs_begin_figure = true;
        }
    }

    void fill() override {
        apply_pending_state();
        close_geometry_sink();
        if (path_geometry && current_brush) {
            rt->FillGeometry (path_geometry, current_brush);
        }
    }

    void stroke() override {
        apply_pending_state();
        close_geometry_sink();
        if (path_geometry && current_brush) {
            rt->DrawGeometry (path_geometry, current_brush, state.line_width);
        }
    }

    void translate (double x, double y) override {
        D2D1_MATRIX_3X2_F transform;
        rt->GetTransform (&transform);
        transform = transform * D2D1::Matrix3x2F::Translation (static_cast<FLOAT> (x), static_cast<FLOAT> (y));
        rt->SetTransform (transform);
        state.clip.x -= x;
        state.clip.y -= y;
    }

    void transform (const Transform& mat) override {
        D2D1_MATRIX_3X2_F d2dMatrix = D2D1::Matrix3x2F (
            static_cast<FLOAT> (mat.m00), static_cast<FLOAT> (mat.m10), static_cast<FLOAT> (mat.m01), static_cast<FLOAT> (mat.m11), static_cast<FLOAT> (mat.m02), static_cast<FLOAT> (mat.m12));

        D2D1_MATRIX_3X2_F current;
        rt->GetTransform (&current);
        rt->SetTransform (current * d2dMatrix);
    }

    void reset_clip() noexcept {
        state.clip = {};
        rt->PopAxisAlignedClip();
    }

    void clip (const Rectangle<int>& r) override {
        state.clip = r.as<double>();
        rt->PushAxisAlignedClip (
            D2D1::RectF (
                static_cast<FLOAT> (r.x),
                static_cast<FLOAT> (r.y),
                static_cast<FLOAT> (r.x + r.width),
                static_cast<FLOAT> (r.y + r.height)),
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }

    void exclude_clip (const Rectangle<int>& r) override {
        // TODO: Implement clip exclusion for Direct2D
        lui::ignore (r);
    }

    Rectangle<int> last_clip() const override {
        return state.clip.as<int>();
    }

    Font font() const noexcept override { return state.font; }

    void set_font (const Font& f) override {
        state.font = f;
        font_dirty = true;
    }

    void set_fill (const Fill& fill) override {
        auto c = state.color = fill.color();
        brush_dirty          = false;

        // Release old brush
        if (current_brush) {
            current_brush->Release();
            current_brush = nullptr;
        }

        // Create new solid color brush
        rt->CreateSolidColorBrush (
            D2D1::ColorF (c.fred(), c.fgreen(), c.fblue(), c.alpha()),
            &current_brush);
    }

    void fill_rect (const Rectangle<double>& r) override {
        apply_pending_state();
        if (current_brush) {
            rt->FillRectangle (
                D2D1::RectF (
                    static_cast<FLOAT> (r.x),
                    static_cast<FLOAT> (r.y),
                    static_cast<FLOAT> (r.x + r.width),
                    static_cast<FLOAT> (r.y + r.height)),
                current_brush);
        }
    }

    FontMetrics font_metrics() const noexcept override {
        FontMetrics fm;

        IDWriteTextFormat* format = create_text_format();
        if (format) {
            // Get font collection
            IDWriteFontCollection* fontCollection = nullptr;
            format->GetFontCollection (&fontCollection);

            if (fontCollection) {
                // Get font family
                WCHAR familyName[100];
                format->GetFontFamilyName (familyName, 100);

                UINT32 index;
                BOOL exists;
                if (SUCCEEDED (fontCollection->FindFamilyName (familyName, &index, &exists)) && exists) {
                    IDWriteFontFamily* fontFamily = nullptr;
                    if (SUCCEEDED (fontCollection->GetFontFamily (index, &fontFamily))) {
                        IDWriteFont* font = nullptr;
                        if (SUCCEEDED (fontFamily->GetFirstMatchingFont (
                                DWRITE_FONT_WEIGHT_NORMAL,
                                DWRITE_FONT_STRETCH_NORMAL,
                                DWRITE_FONT_STYLE_NORMAL,
                                &font))) {
                            DWRITE_FONT_METRICS metrics;
                            font->GetMetrics (&metrics);

                            float fontSize = format->GetFontSize();
                            float scale    = fontSize / metrics.designUnitsPerEm;

                            fm.ascent       = metrics.ascent * scale;
                            fm.descent      = metrics.descent * scale;
                            fm.height       = fm.ascent + fm.descent;
                            fm.x_stride_max = metrics.maxWidth * scale;
                            fm.y_stride_max = (metrics.ascent + metrics.descent) * scale;

                            font->Release();
                        }
                        fontFamily->Release();
                    }
                }
                fontCollection->Release();
            }
            format->Release();
        }

        return fm;
    }

    TextMetrics text_metrics (std::string_view text) const noexcept override {
        TextMetrics tm;

        IDWriteTextFormat* format = create_text_format();
        if (! format)
            return tm;

        // Convert UTF-8 to wide string
        int wlen = MultiByteToWideChar (CP_UTF8, 0, text.data(), (int) text.size(), nullptr, 0);
        if (wlen <= 0) {
            format->Release();
            return tm;
        }

        std::vector<wchar_t> wtext (wlen + 1);
        MultiByteToWideChar (CP_UTF8, 0, text.data(), (int) text.size(), wtext.data(), wlen);
        wtext[wlen] = 0;

        // Get DWrite factory
        IDWriteFactory* writeFactory = get_write_factory();
        if (! writeFactory) {
            format->Release();
            return tm;
        }

        // Create text layout
        IDWriteTextLayout* textLayout = nullptr;
        HRESULT hr                    = writeFactory->CreateTextLayout (
            wtext.data(),
            wlen,
            format,
            10000.0f, // Max width
            10000.0f, // Max height
            &textLayout);

        if (SUCCEEDED (hr)) {
            DWRITE_TEXT_METRICS metrics;
            textLayout->GetMetrics (&metrics);

            tm.width    = metrics.width;
            tm.height   = metrics.height;
            tm.x_offset = metrics.left;
            tm.y_offset = metrics.top;
            tm.x_stride = metrics.widthIncludingTrailingWhitespace;
            tm.y_stride = 0.0;

            textLayout->Release();
        }

        format->Release();
        return tm;
    }

    bool show_text (const std::string_view text) override {
        apply_pending_state();

        // Convert UTF-8 to wide string
        int wlen = MultiByteToWideChar (CP_UTF8, 0, text.data(), (int) text.size(), nullptr, 0);
        if (wlen <= 0)
            return false;

        std::vector<wchar_t> wtext (wlen + 1);
        MultiByteToWideChar (CP_UTF8, 0, text.data(), (int) text.size(), wtext.data(), wlen);
        wtext[wlen] = 0;

        IDWriteTextFormat* format = create_text_format();
        if (! format)
            return false;

        // Get DWrite factory
        IDWriteFactory* writeFactory = get_write_factory();
        if (! writeFactory) {
            format->Release();
            return false;
        }

        // Create text layout
        IDWriteTextLayout* textLayout = nullptr;
        HRESULT hr                    = writeFactory->CreateTextLayout (
            wtext.data(),
            wlen,
            format,
            10000.0f,
            10000.0f,
            &textLayout);

        if (SUCCEEDED (hr) && current_brush) {
            // Draw at current point
            rt->DrawTextLayout (
                current_point,
                textLayout,
                current_brush,
                D2D1_DRAW_TEXT_OPTIONS_NONE);

            textLayout->Release();
            format->Release();
            return true;
        }

        if (textLayout)
            textLayout->Release();
        format->Release();
        return false;
    }

    void draw_image (Image i, Transform matrix) override {
        // Create D2D bitmap from image data
        D2D1_BITMAP_PROPERTIES props = {
            { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
            96.0f,
            96.0f
        };

        ID2D1Bitmap* bitmap = nullptr;
        HRESULT hr          = rt->CreateBitmap (
            D2D1::SizeU (i.width(), i.height()),
            i.data(),
            i.stride(),
            props,
            &bitmap);

        if (SUCCEEDED (hr) && bitmap) {
            D2D1_MATRIX_3X2_F oldTransform;
            rt->GetTransform (&oldTransform);

            transform (matrix);

            rt->DrawBitmap (
                bitmap,
                D2D1::RectF (0, 0, (FLOAT) i.width(), (FLOAT) i.height()),
                1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

            rt->SetTransform (oldTransform);
            bitmap->Release();
        }
    }

private:
    void ensure_path() {
        if (! path_geometry) {
            clear_path();
        }
    }

    void ensure_figure() {
        if (needs_begin_figure && geometry_sink) {
            geometry_sink->BeginFigure (current_point, D2D1_FIGURE_BEGIN_FILLED);
            needs_begin_figure = false;
        }
    }

    void close_geometry_sink() {
        if (geometry_sink) {
            if (! needs_begin_figure) {
                geometry_sink->EndFigure (D2D1_FIGURE_END_OPEN);
                needs_begin_figure = true;
            }
            geometry_sink->Close();
            geometry_sink->Release();
            geometry_sink = nullptr;
        }
    }

    void apply_pending_state() {
        if (brush_dirty) {
            set_fill (Fill { state.color });
        }
        if (font_dirty) {
            font_dirty = false;
        }
    }

    IDWriteTextFormat* create_text_format() const {
        IDWriteFactory* writeFactory = get_write_factory();
        if (! writeFactory)
            return nullptr;

        IDWriteTextFormat* format = nullptr;
        writeFactory->CreateTextFormat (
            LUI_D2D_DEFAULT_FONT,
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            static_cast<FLOAT> (state.font.height()),
            L"en-us",
            &format);

        return format;
    }

    IDWriteFactory* get_write_factory() const {
        // This should be stored with the surface/view
        // For now, create a shared instance
        static IDWriteFactory* factory = nullptr;
        if (! factory) {
            DWriteCreateFactory (
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof (IDWriteFactory),
                reinterpret_cast<IUnknown**> (&factory));
        }
        return factory;
    }

    void release_resources() {
        if (geometry_sink) {
            geometry_sink->Release();
            geometry_sink = nullptr;
        }
        if (path_geometry) {
            path_geometry->Release();
            path_geometry = nullptr;
        }
        if (current_brush) {
            current_brush->Release();
            current_brush = nullptr;
        }
        if (current_layer) {
            current_layer->Release();
            current_layer = nullptr;
        }
    }

    struct State {
        Font font;
        Color color;
        Rectangle<double> clip;
        FLOAT line_width { 1.0f };
    };

    ID2D1RenderTarget* rt { nullptr };
    ID2D1PathGeometry* path_geometry { nullptr };
    ID2D1GeometrySink* geometry_sink { nullptr };
    ID2D1SolidColorBrush* current_brush { nullptr };
    ID2D1Layer* current_layer { nullptr };
    D2D1_POINT_2F current_point { 0, 0 };
    bool needs_begin_figure { true };

    State state;
    std::vector<State> stack;
    bool brush_dirty { false };
    bool font_dirty { false };
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
        auto rt = (ID2D1RenderTarget*) puglGetContext (_view);
        assert (rt != nullptr);

        if (true || ! _scale_set || _last_scale != scale_factor()) {
            _scale_set         = true;
            const auto scale_x = scale_factor();
            const auto scale_y = scale_x;
            _last_scale        = scale_x;
        }

        if (_context->begin_frame (rt, frame)) {
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
    bool _scale_set { false };
    double _last_scale { 1.0 };
};

} // namespace d2d

std::unique_ptr<lui::View> Direct2D::create_view (Main& c, Widget& w) {
    return std::make_unique<d2d::View> (c, w);
}

} // namespace lui
