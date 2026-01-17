// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

/**
    Coordinate System Strategy:

    GDI uses a top-left origin coordinate system by default, which matches
    the natural LUI coordinate system. Paths, fills, strokes, and rectangles
    work directly with top-left coordinates.

    Text rendering in GDI also uses top-left origin naturally, so no
    coordinate flipping is needed for text operations.
*/

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <windows.h>
#include <wingdi.h>

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
    }

    ~Context() {
        release_resources();
    }

    bool begin_frame (HDC _dc, lui::Bounds bounds) {
        dc    = _dc;
        state = {};
        stack.clear();
        release_resources();

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
        SaveDC (dc);
    }

    void restore() override {
        RestoreDC (dc, -1);
        if (stack.empty())
            return;
        std::swap (state, stack.back());
        stack.pop_back();
    }

    void set_line_width (double width) override {
        state.line_width = static_cast<int> (width);
    }

    void clear_path() override {
        path_points.clear();
        path_types.clear();
        current_pos = { 0, 0 };
    }

    void move_to (double x1, double y1) override {
        path_points.push_back (POINT { static_cast<LONG> (x1), static_cast<LONG> (y1) });
        path_types.push_back (PT_MOVETO);
        current_pos = { static_cast<LONG> (x1), static_cast<LONG> (y1) };
    }

    void line_to (double x1, double y1) override {
        path_points.push_back (POINT { static_cast<LONG> (x1), static_cast<LONG> (y1) });
        path_types.push_back (PT_LINETO);
        current_pos = { static_cast<LONG> (x1), static_cast<LONG> (y1) };
    }

    void quad_to (double x1, double y1, double x2, double y2) override {
        // Convert quadratic bezier to cubic
        // Control points: Q1 = current, Q2 = (x1,y1), Q3 = (x2,y2)
        // Cubic: P0 = Q1, P1 = Q1 + 2/3(Q2-Q1), P2 = Q3 + 2/3(Q2-Q3), P3 = Q3
        double cx1 = current_pos.x + 2.0 / 3.0 * (x1 - current_pos.x);
        double cy1 = current_pos.y + 2.0 / 3.0 * (y1 - current_pos.y);
        double cx2 = x2 + 2.0 / 3.0 * (x1 - x2);
        double cy2 = y2 + 2.0 / 3.0 * (y1 - y2);

        cubic_to (cx1, cy1, cx2, cy2, x2, y2);
    }

    void cubic_to (double x1, double y1, double x2, double y2, double x3, double y3) override {
        // GDI uses PolyBezierTo which requires control points and end point
        path_points.push_back (POINT { static_cast<LONG> (x1), static_cast<LONG> (y1) });
        path_types.push_back (PT_BEZIERTO);
        path_points.push_back (POINT { static_cast<LONG> (x2), static_cast<LONG> (y2) });
        path_types.push_back (PT_BEZIERTO);
        path_points.push_back (POINT { static_cast<LONG> (x3), static_cast<LONG> (y3) });
        path_types.push_back (PT_BEZIERTO);
        current_pos = { static_cast<LONG> (x3), static_cast<LONG> (y3) };
    }

    void close_path() override {
        if (! path_types.empty()) {
            path_types.back() |= PT_CLOSEFIGURE;
        }
    }

    void fill() override {
        apply_pending_state();
        if (path_points.empty())
            return;

        BeginPath (dc);
        PolyDraw (dc, path_points.data(), path_types.data(), static_cast<int> (path_points.size()));
        EndPath (dc);

        if (current_brush) {
            SelectObject (dc, current_brush);
            FillPath (dc);
        }
    }

    void stroke() override {
        apply_pending_state();
        if (path_points.empty())
            return;

        BeginPath (dc);
        PolyDraw (dc, path_points.data(), path_types.data(), static_cast<int> (path_points.size()));
        EndPath (dc);

        if (current_pen) {
            SelectObject (dc, current_pen);
            StrokePath (dc);
        }
    }

    void translate (double x, double y) override {
        XFORM xform;
        xform.eM11 = 1.0f;
        xform.eM12 = 0.0f;
        xform.eM21 = 0.0f;
        xform.eM22 = 1.0f;
        xform.eDx  = static_cast<FLOAT> (x);
        xform.eDy  = static_cast<FLOAT> (y);

        ModifyWorldTransform (dc, &xform, MWT_LEFTMULTIPLY);

        state.clip.x -= x;
        state.clip.y -= y;
    }

    void transform (const Transform& mat) override {
        XFORM xform;
        xform.eM11 = static_cast<FLOAT> (mat.m00);
        xform.eM12 = static_cast<FLOAT> (mat.m10);
        xform.eM21 = static_cast<FLOAT> (mat.m01);
        xform.eM22 = static_cast<FLOAT> (mat.m11);
        xform.eDx  = static_cast<FLOAT> (mat.m02);
        xform.eDy  = static_cast<FLOAT> (mat.m12);

        ModifyWorldTransform (dc, &xform, MWT_LEFTMULTIPLY);
    }

    void clip (const Rectangle<int>& r) override {
        state.clip = r.as<double>();
        HRGN rgn   = CreateRectRgn (r.x, r.y, r.x + r.width, r.y + r.height);
        SelectClipRgn (dc, rgn);
        DeleteObject (rgn);
    }

    void exclude_clip (const Rectangle<int>& r) override {
        HRGN rgn = CreateRectRgn (r.x, r.y, r.x + r.width, r.y + r.height);
        ExtSelectClipRgn (dc, rgn, RGN_DIFF);
        DeleteObject (rgn);
    }

    Rectangle<int> last_clip() const override {
        return state.clip.as<int>();
    }

    Font font() const noexcept override {
        return state.font;
    }

    void set_font (const Font& font) override {
        state.font = font;
        font_dirty = true;
    }

    void set_fill (const Fill& fill) override {
        if (fill.is_color()) {
            state.color = fill.color();
            brush_dirty = true;
        }
    }

    void fill_rect (const Rectangle<double>& r) override {
        apply_pending_state();
        RECT rect;
        rect.left   = static_cast<LONG> (r.x);
        rect.top    = static_cast<LONG> (r.y);
        rect.right  = static_cast<LONG> (r.x + r.width);
        rect.bottom = static_cast<LONG> (r.y + r.height);

        if (current_brush) {
            FillRect (dc, &rect, current_brush);
        }
    }

    FontMetrics font_metrics() const noexcept override {
        TEXTMETRIC tm;
        GetTextMetrics (dc, &tm);
        return {
            static_cast<double> (tm.tmAscent),
            static_cast<double> (tm.tmDescent),
            static_cast<double> (tm.tmHeight),
            static_cast<double> (tm.tmAveCharWidth),
            static_cast<double> (tm.tmHeight)
        };
    }

    TextMetrics text_metrics (std::string_view text) const noexcept override {
        SIZE size;
        GetTextExtentPoint32A (dc, text.data(), static_cast<int> (text.length()), &size);
        return {
            static_cast<double> (size.cx),
            static_cast<double> (size.cy),
            0.0,
            0.0,
            static_cast<double> (size.cx),
            static_cast<double> (size.cy)
        };
    }

    bool show_text (std::string_view text) override {
        apply_pending_state();

        SetBkMode (dc, TRANSPARENT);
        auto c = state.color;
        SetTextColor (dc, RGB (c.red(), c.green(), c.blue()));

        TextOutA (dc, current_pos.x, current_pos.y, text.data(), static_cast<int> (text.length()));
        return true;
    }

    void draw_image (Image i, Transform matrix) override {
        // Create a memory DC for the source image
        HDC memDC = CreateCompatibleDC (dc);
        if (! memDC)
            return;

        // Create a DIB section for the image
        BITMAPINFO bmi              = {};
        bmi.bmiHeader.biSize        = sizeof (BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth       = i.width();
        bmi.bmiHeader.biHeight      = -i.height(); // Negative for top-down DIB
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* bits      = nullptr;
        HBITMAP hBitmap = CreateDIBSection (memDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (! hBitmap) {
            DeleteDC (memDC);
            return;
        }

        // Copy image data
        memcpy (bits, i.data(), i.stride() * i.height());

        HBITMAP oldBitmap = (HBITMAP) SelectObject (memDC, hBitmap);

        // Save current transform
        XFORM oldXform;
        GetWorldTransform (dc, &oldXform);

        // Apply the transformation matrix
        transform (matrix);

        // Blit the image
        BitBlt (dc, 0, 0, i.width(), i.height(), memDC, 0, 0, SRCCOPY);

        // Restore transform
        SetWorldTransform (dc, &oldXform);

        SelectObject (memDC, oldBitmap);
        DeleteObject (hBitmap);
        DeleteDC (memDC);
    }

private:
    void apply_pending_state() {
        if (brush_dirty) {
            if (current_brush) {
                DeleteObject (current_brush);
                current_brush = nullptr;
            }
            auto c        = state.color;
            current_brush = CreateSolidBrush (RGB (c.red(), c.green(), c.blue()));
            brush_dirty   = false;
        }

        if (pen_dirty || state.line_width != last_line_width) {
            if (current_pen) {
                DeleteObject (current_pen);
                current_pen = nullptr;
            }
            auto c          = state.color;
            current_pen     = CreatePen (PS_SOLID, state.line_width, RGB (c.red(), c.green(), c.blue()));
            pen_dirty       = false;
            last_line_width = state.line_width;
        }

        if (font_dirty) {
            if (current_font) {
                DeleteObject (current_font);
                current_font = nullptr;
            }

            current_font = CreateFontA (
                static_cast<int> (state.font.height()),
                0,
                0,
                0,
                FW_NORMAL,
                FALSE,
                FALSE,
                FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE,
                "Arial");

            if (current_font) {
                SelectObject (dc, current_font);
            }
            font_dirty = false;
        }
    }

    void release_resources() {
        if (current_brush) {
            DeleteObject (current_brush);
            current_brush = nullptr;
        }
        if (current_pen) {
            DeleteObject (current_pen);
            current_pen = nullptr;
        }
        if (current_font) {
            DeleteObject (current_font);
            current_font = nullptr;
        }
        path_points.clear();
        path_types.clear();
    }

    struct State {
        Font font;
        Color color;
        Rectangle<double> clip;
        int line_width { 1 };
    };

    HDC dc { nullptr };
    HBRUSH current_brush { nullptr };
    HPEN current_pen { nullptr };
    HFONT current_font { nullptr };
    POINT current_pos { 0, 0 };

    std::vector<POINT> path_points;
    std::vector<BYTE> path_types;

    State state;
    std::vector<State> stack;
    bool brush_dirty { false };
    bool pen_dirty { false };
    bool font_dirty { false };
    int last_line_width { 1 };
};

class View : public lui::View {
public:
    View (Main& m, Widget& w)
        : lui::View (m, w) {
        set_backend ((uintptr_t) puglGdiBackend());
        set_view_hint (PUGL_DOUBLE_BUFFER, PUGL_FALSE);
        set_view_hint (PUGL_RESIZABLE, PUGL_TRUE);
        puglSetViewString ((PuglView*) c_obj(), PUGL_WINDOW_TITLE, w.name().c_str());
    }

    ~View() {}

    void expose (Bounds frame) override {
        auto hdc = (HDC) puglGetContext (_view);
        assert (hdc != nullptr);

        if (true || ! _scale_set || _last_scale != scale_factor()) {
            _scale_set         = true;
            const auto scale_x = scale_factor();
            const auto scale_y = scale_x;
            _last_scale        = scale_x;

            // Set advanced graphics mode for transformations
            SetGraphicsMode (hdc, GM_ADVANCED);
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
    bool _scale_set { false };
    double _last_scale { 1.0 };
};

} // namespace gdi

std::unique_ptr<lui::View> GDI::create_view (Main& c, Widget& w) {
    return std::make_unique<gdi::View> (c, w);
}

} // namespace lui
