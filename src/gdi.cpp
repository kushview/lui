// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

/**
    Coordinate System Strategy:

    GDI+ uses a top-left origin coordinate system by default, which matches
    the natural LUI coordinate system. Paths, fills, strokes, and rectangles
    work directly with top-left coordinates.

    Text rendering in GDI+ also uses top-left origin naturally, so no
    coordinate flipping is needed for text operations.
*/

#include "Roboto-Regular.h"
#include <cassert>
#include <cmath>
#include <windows.h>
#include <oleauto.h>
#include <gdiplus.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <lui/gdi.hpp>
#include <lui/graphics.hpp>
#include <lui/widget.hpp>

#include "pugl/src/stub.h"

extern "C" {
const PuglBackend* puglGdiBackend();
}

namespace lui {
namespace gdi {

class Context : public DrawingContext {
public:
    explicit Context (HDC hdc = nullptr)
        : dc (hdc) {
        stack.reserve (64);

        // Initialize GDI+
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup (&gdiplusToken, &gdiplusStartupInput, nullptr);

        // Load embedded Roboto font from memory using GDI+ PrivateFontCollection
        fontCollection = std::make_unique<Gdiplus::PrivateFontCollection>();
        Gdiplus::Status status = fontCollection->AddMemoryFont (
            (void*) Roboto_Regular_ttf,
            sizeof (Roboto_Regular_ttf));
        
        if (status != Gdiplus::Ok) {
            std::cerr << "Failed to load Roboto font, status: " << status << std::endl;
        }
    }

    ~Context() {
        release_resources();

        // Font collection will be cleaned up automatically
        fontCollection.reset();

        // Shutdown GDI+
        Gdiplus::GdiplusShutdown (gdiplusToken);
    }

    bool begin_frame (HDC _dc, lui::Bounds bounds) {
        dc         = _dc;
        state      = {};
        state.font = Font (14.0f);
        stack.clear();
        release_resources();

        // Create GDI+ Graphics object
        graphics = std::make_unique<Gdiplus::Graphics> (dc);
        graphics->SetSmoothingMode (Gdiplus::SmoothingModeHighQuality);
        graphics->SetTextRenderingHint (Gdiplus::TextRenderingHintAntiAliasGridFit);
        graphics->SetCompositingQuality (Gdiplus::CompositingQualityHighQuality);
        graphics->SetInterpolationMode (Gdiplus::InterpolationModeHighQualityBicubic);

        // Create path and reset position
        path = std::make_unique<Gdiplus::GraphicsPath>();
        current_pos = Gdiplus::PointF (0, 0);

        this->clip (bounds);
        return true;
    }

    void end_frame() {
        release_resources();
        dc = nullptr;
    }

    double device_scale() const noexcept override {
        assert (dc != nullptr);
        int logPixelsY = GetDeviceCaps (dc, LOGPIXELSY);
        return static_cast<double> (logPixelsY / 96.0); // 96 DPI is default
    }

    void save() override {
        stack.push_back (state);
        if (graphics) {
            graphics_states.push_back (graphics->Save());
        }
    }

    void restore() override {
        if (graphics && ! graphics_states.empty()) {
            graphics->Restore (graphics_states.back());
            graphics_states.pop_back();
        }
        if (stack.empty())
            return;
        std::swap (state, stack.back());
        stack.pop_back();
    }

    void set_line_width (double width) override {
        state.line_width = static_cast<float> (width);
    }

    void clear_path() override {
        path        = std::make_unique<Gdiplus::GraphicsPath>();
        current_pos = Gdiplus::PointF (0, 0);
    }

    void move_to (double x1, double y1) override {
        path->StartFigure();
        current_pos = Gdiplus::PointF (static_cast<float> (x1), static_cast<float> (y1));
    }

    void line_to (double x1, double y1) override {
        path->AddLine (current_pos, Gdiplus::PointF (static_cast<float> (x1), static_cast<float> (y1)));
        current_pos = Gdiplus::PointF (static_cast<float> (x1), static_cast<float> (y1));
    }

    void quad_to (double x1, double y1, double x2, double y2) override {
        // Convert quadratic bezier to cubic
        double cx1 = current_pos.X + 2.0 / 3.0 * (x1 - current_pos.X);
        double cy1 = current_pos.Y + 2.0 / 3.0 * (y1 - current_pos.Y);
        double cx2 = x2 + 2.0 / 3.0 * (x1 - x2);
        double cy2 = y2 + 2.0 / 3.0 * (y1 - y2);

        cubic_to (cx1, cy1, cx2, cy2, x2, y2);
    }

    void cubic_to (double x1, double y1, double x2, double y2, double x3, double y3) override {
        path->AddBezier (
            current_pos,
            Gdiplus::PointF (static_cast<float> (x1), static_cast<float> (y1)),
            Gdiplus::PointF (static_cast<float> (x2), static_cast<float> (y2)),
            Gdiplus::PointF (static_cast<float> (x3), static_cast<float> (y3)));
        current_pos = Gdiplus::PointF (static_cast<float> (x3), static_cast<float> (y3));
    }

    void close_path() override {
        path->CloseFigure();
    }

    void fill() override {
        if (! graphics || ! path)
            return;

        auto c = state.color;
        Gdiplus::SolidBrush brush (Gdiplus::Color (c.alpha(), c.red(), c.green(), c.blue()));
        graphics->FillPath (&brush, path.get());
    }

    void stroke() override {
        if (! graphics || ! path)
            return;

        auto c = state.color;
        Gdiplus::Pen pen (Gdiplus::Color (c.alpha(), c.red(), c.green(), c.blue()), state.line_width);
        graphics->DrawPath (&pen, path.get());
    }

    void translate (double x, double y) override {
        if (graphics) {
            graphics->TranslateTransform (static_cast<float> (x), static_cast<float> (y));
        }
        state.clip.x -= x;
        state.clip.y -= y;
    }

    void transform (const Transform& mat) override {
        if (graphics) {
            Gdiplus::Matrix matrix (
                static_cast<float> (mat.m00),
                static_cast<float> (mat.m10),
                static_cast<float> (mat.m01),
                static_cast<float> (mat.m11),
                static_cast<float> (mat.m02),
                static_cast<float> (mat.m12));
            graphics->MultiplyTransform (&matrix);
        }
    }

    void clip (const Rectangle<int>& r) override {
        state.clip = r.as<double>();
        if (graphics) {
            graphics->SetClip (Gdiplus::Rect (r.x, r.y, r.width, r.height));
        }
    }

    void exclude_clip (const Rectangle<int>&) override {
#if 0 // TODO:
        if (graphics) {
            graphics->ExcludeClip (Gdiplus::Rect (r.x, r.y, r.width, r.height));
        }
#endif
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
        if (! graphics)
            return;

        auto c = state.color;
        Gdiplus::SolidBrush brush (Gdiplus::Color (c.alpha(), c.red(), c.green(), c.blue()));
        graphics->FillRectangle (
            &brush,
            static_cast<float> (r.x),
            static_cast<float> (r.y),
            static_cast<float> (r.width),
            static_cast<float> (r.height));
    }

    FontMetrics font_metrics() const noexcept override {
        if (! graphics || ! fontCollection)
            return {};

        // Get font family from private collection
        Gdiplus::FontFamily fontFamily;
        int found = 0;
        fontCollection->GetFamilies (1, &fontFamily, &found);
        if (found == 0)
            return {};

        Gdiplus::Font gdiFont (&fontFamily, state.font.height(), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

        const float emSize   = gdiFont.GetSize();
        const float cellAsc  = fontFamily.GetCellAscent (Gdiplus::FontStyleRegular);
        const float cellDesc = fontFamily.GetCellDescent (Gdiplus::FontStyleRegular);
        const float emHeight = fontFamily.GetEmHeight (Gdiplus::FontStyleRegular);

        const float ascent  = emSize * cellAsc / emHeight;
        const float descent = emSize * cellDesc / emHeight;
        const float height  = ascent + descent;

        return {
            static_cast<double> (ascent),
            static_cast<double> (descent),
            static_cast<double> (height),
            static_cast<double> (emSize / 2.0), // approximate
            static_cast<double> (height)
        };
    }

    TextMetrics text_metrics (std::string_view text) const noexcept override {
        if (! graphics || ! fontCollection)
            return {};

        // Get font family from private collection
        Gdiplus::FontFamily fontFamily;
        int found = 0;
        fontCollection->GetFamilies (1, &fontFamily, &found);
        if (found == 0)
            return {};

        std::wstring wtext (text.begin(), text.end());
        Gdiplus::Font gdiFont (&fontFamily, state.font.height(), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::RectF layoutRect;
        Gdiplus::RectF boundingBox;
        graphics->MeasureString (wtext.c_str(), -1, &gdiFont, layoutRect, &boundingBox);

        return {
            static_cast<double> (boundingBox.Width),
            static_cast<double> (boundingBox.Height),
            0.0,
            0.0,
            static_cast<double> (boundingBox.Width),
            static_cast<double> (boundingBox.Height)
        };
    }

    bool show_text (std::string_view text) override {
        if (! graphics || ! fontCollection)
            return false;

        std::wstring wtext (text.begin(), text.end());
        
        // Get the font family from our private collection
        int numFamilies = fontCollection->GetFamilyCount();
        if (numFamilies == 0)
            return false;
            
        Gdiplus::FontFamily fontFamily;
        int found = 0;
        fontCollection->GetFamilies (1, &fontFamily, &found);
        
        if (found == 0)
            return false;

        Gdiplus::Font gdiFont (&fontFamily, state.font.height(), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

        // Get font metrics to calculate baseline adjustment
        const float emSize   = gdiFont.GetSize();
        const float cellAsc  = fontFamily.GetCellAscent (Gdiplus::FontStyleRegular);
        const float emHeight = fontFamily.GetEmHeight (Gdiplus::FontStyleRegular);
        const float ascent   = emSize * cellAsc / emHeight;

        auto c = state.color;
        Gdiplus::SolidBrush brush (Gdiplus::Color (c.alpha(), c.red(), c.green(), c.blue()));

        // GDI+ DrawString uses top-left positioning, but current_pos represents the baseline
        // Adjust Y coordinate by subtracting ascent to position text correctly
        Gdiplus::PointF origin (current_pos.X, current_pos.Y - ascent);

        graphics->DrawString (wtext.c_str(), -1, &gdiFont, origin, &brush);
        return true;
    }

    void draw_image (Image i, Transform matrix) override {
        if (! graphics)
            return;

        // Create a GDI+ bitmap from the image data
        Gdiplus::Bitmap bitmap (i.width(), i.height(), i.stride(), PixelFormat32bppARGB, (BYTE*) i.data());

        // Save current transform
        Gdiplus::GraphicsState state = graphics->Save();

        // Apply the transformation matrix
        Gdiplus::Matrix mat (
            static_cast<float> (matrix.m00),
            static_cast<float> (matrix.m10),
            static_cast<float> (matrix.m01),
            static_cast<float> (matrix.m11),
            static_cast<float> (matrix.m02),
            static_cast<float> (matrix.m12));
        graphics->MultiplyTransform (&mat);

        // Draw the image
        graphics->DrawImage (&bitmap, 0, 0, i.width(), i.height());

        // Restore transform
        graphics->Restore (state);
    }

private:
    void release_resources() {
        graphics.reset();
        path.reset();
        graphics_states.clear();
    }

    struct State {
        Font font;
        Color color;
        Rectangle<double> clip;
        float line_width { 1.0f };
    };

    HDC dc { nullptr };
    ULONG_PTR gdiplusToken { 0 };
    std::unique_ptr<Gdiplus::Graphics> graphics;
    std::unique_ptr<Gdiplus::GraphicsPath> path;
    std::unique_ptr<Gdiplus::PrivateFontCollection> fontCollection;
    Gdiplus::PointF current_pos { 0, 0 };

    State state;
    std::vector<State> stack;
    std::vector<Gdiplus::GraphicsState> graphics_states;
};

class View : public lui::View {
public:
    View (Main& m, Widget& w)
        : lui::View (m, w) {
        set_backend ((uintptr_t) puglGdiBackend());
        set_view_hint (PUGL_DOUBLE_BUFFER, PUGL_TRUE);
        set_view_hint (PUGL_RESIZABLE, PUGL_TRUE);
        puglSetViewString ((PuglView*) c_obj(), PUGL_WINDOW_TITLE, w.name().c_str());
    }

    ~View() {}

    void expose (Bounds frame) override {
        auto hdc = (HDC) puglGetContext (_view);
        assert (hdc != nullptr);

        // Set advanced graphics mode for transformations
        SetGraphicsMode (hdc, GM_ADVANCED);

        const auto scale = 1.0 / scale_factor();
        if (scale != 1.0) {
            // Apply DPI scaling transform
            XFORM xform;
            xform.eM11 = static_cast<FLOAT> (scale);
            xform.eM12 = 0.0f;
            xform.eM21 = 0.0f;
            xform.eM22 = static_cast<FLOAT> (scale);
            xform.eDx  = 0.0f;
            xform.eDy  = 0.0f;
            SetWorldTransform (hdc, &xform);
        }

        if (_context->begin_frame (hdc, frame)) {
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
};

} // namespace gdi

std::unique_ptr<lui::View> GDI::create_view (Main& c, Widget& w) {
    return std::make_unique<gdi::View> (c, w);
}

} // namespace lui
