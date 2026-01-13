// Copyright 2024 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <cassert>
#include <iostream>

#if _MSC_VER
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <cairo-win32.h>
#    undef NOMINMAX
#elif __APPLE__
#    include <cairo-quartz.h>
#else
#    include <cairo.h>
#endif

#include <pugl/cairo.h>

#include <lui/cairo.hpp>
#include <lui/widget.hpp>

namespace lui {
namespace cairo {

class Context : public DrawingContext {
public:
    explicit Context (cairo_t* context = nullptr)
        : cr (context) {
        stack.reserve (64);
    }

    ~Context() {
        cr = nullptr;
    }

    bool begin_frame (cairo_t* _cr, lui::Bounds bounds) {
        cr    = _cr;
        state = {};
        stack.clear();
        this->clip (bounds);
        return true;
    }

    void end_frame() {
        cr = nullptr;
    }

    double device_scale() const noexcept override {
        assert (cr != nullptr);
        double x_scale = 1.0, y_scale = 1.0;
        if (auto s = cairo_get_target (cr))
            cairo_surface_get_device_scale (s, &x_scale, &y_scale);
        return static_cast<double> (y_scale);
    }

    void save() override {
        cairo_save (cr);
        stack.push_back (state);
    }

    void restore() override {
        cairo_restore (cr);
        if (stack.empty())
            return;
        std::swap (state, stack.back());
        stack.pop_back();
    }

    void set_line_width (double width) override {
        cairo_set_line_width (cr, width);
    }

    void clear_path() override {
        cairo_new_path (cr);
    }

    void move_to (double x1, double y1) override {
        cairo_move_to (cr, x1, y1);
    }

    void line_to (double x1, double y1) override {
        cairo_line_to (cr, x1, y1);
    }

    void quad_to (double x1, double y1, double x2, double y2) override {
        double x0, y0;
        cairo_get_current_point (cr, &x0, &y0);
        cairo_curve_to (cr,
                        (x0 + 2.0 * x1) / 3.0,
                        (y0 + 2.0 * y1) / 3.0,
                        (x2 + 2.0 * x1) / 3.0,
                        (y2 + 2.0 * y1) / 3.0,
                        x2,
                        y2);
    }

    void cubic_to (double x1, double y1, double x2, double y2, double x3, double y3) override {
        cairo_curve_to (cr, x1, y1, x2, y2, x3, y3);
    }

    void close_path() override {
        cairo_close_path (cr);
    }

    /** Fill the current path with the currrent settings */
    void fill() override {
        apply_pending_state();
        cairo_fill (cr);
    }

    /** Stroke the current path with current settings */
    void stroke() override {
        apply_pending_state();
        cairo_stroke (cr);
    }

    /** Translate the origin */
    void translate (double x, double y) override {
        cairo_translate (cr, x, y);
        state.clip.x -= x;
        state.clip.y -= y;
    }

    /** Apply transformation matrix */
    void transform (const Transform& mat) override {
        // clang-format off
        cairo_matrix_t m = { mat.m00, mat.m10, 
                             mat.m01, mat.m11, 
                             mat.m02, mat.m12 };
        // clang-format on
        cairo_transform (cr, &m);
    }

    void reset_clip() noexcept {
        state.clip = {};
        cairo_reset_clip (cr);
    }

    void clip (const Rectangle<int>& r) override {
        state.clip = r.as<double>();
        cairo_new_path (cr);
        cairo_rectangle (cr, r.x, r.y, r.width, r.height);
        cairo_clip (cr);
    }

    void exclude_clip (const Rectangle<int>& r) override {
#if 1 // FIXME: exlusions still aren't quite right.
        lui::ignore (r);
#else
        // Create a clipping region that excludes the specified rectangle
        // Uses even-odd fill rule to create a hole in the clip region
        cairo_new_path (cr);

        // Add outer rectangle (current clip bounds)
        auto current = state.clip;
        cairo_rectangle (cr, current.x, current.y, current.width, current.height);

        // Add inner rectangle (excluded region)
        cairo_rectangle (cr, r.x, r.y, r.width, r.height);

        // Apply even-odd rule to create exclusion, then restore default
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_clip (cr);
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_WINDING);

        // Note: state.clip remains as the outer rectangle since we can't
        // represent a complex clip region as a single rectangle
#endif
    }

    Rectangle<int> last_clip() const override {
        return state.clip.as<int>();
    }

    Font font() const noexcept override { return state.font; }
    void set_font (const Font& f) override {
        // TODO: equals operator is not yet reliable in lui::Font
        // if (state.font == f)
        //     return;
        state.font = f;
        cairo_set_font_size (cr, f.height());
    }

    void set_fill (const Fill& fill) override {
        auto c = state.color = fill.color();
        _fill_dirty          = false;
        cairo_set_source_rgba (cr, c.fred(), c.fgreen(), c.fblue(), c.alpha());
    }

    void fill_rect (const Rectangle<double>& r) override {
        apply_pending_state();
        cairo_rectangle (cr, r.x, r.y, r.width, r.height);
        cairo_fill (cr);
    }

    FontMetrics font_metrics() const noexcept override {
        cairo_font_extents_t cfe;
        cairo_font_extents (cr, &cfe);
        return {
            cfe.ascent,
            cfe.descent,
            cfe.height,
            cfe.max_x_advance,
            cfe.max_y_advance
        };
    }

    TextMetrics text_metrics (std::string_view text) const noexcept override {
        cairo_text_extents_t cte;
        cairo_text_extents (cr, text.data(), &cte);
        return {
            cte.width,
            cte.height,
            cte.x_bearing,
            cte.y_bearing,
            cte.x_advance,
            cte.y_advance
        };
    }

    bool show_text (std::string_view text) override {
        apply_pending_state();
        cairo_show_text (cr, text.data());
        return true;
    }

    void draw_image (Image i, Transform matrix) override {
        cairo_surface_t* image = nullptr;
        cairo_format_t format  = CAIRO_FORMAT_INVALID;

        switch (i.format()) {
            case PixelFormat::ARGB32:
                format = CAIRO_FORMAT_ARGB32;
                break;
            case PixelFormat::RGB24:
                format = CAIRO_FORMAT_RGB24;
                break;
            case PixelFormat::INVALID:
            default:
                format = CAIRO_FORMAT_INVALID;
                break;
        }

        if (format == CAIRO_FORMAT_INVALID) {
            return;
        }

        image = cairo_image_surface_create_for_data (
            i.data(), format, i.width(), i.height(), i.stride());

        if (image == nullptr || 0 != cairo_surface_status (image)) {
            if (image != nullptr)
                cairo_surface_destroy (image);
            return;
        }

        transform (matrix);
        cairo_set_source_surface (cr, image, 0, 0);
        cairo_paint (cr);

        cairo_surface_destroy (image);
    }

private:
    cairo_t* cr { nullptr };
    struct State {
        lui::Color color;
        Rectangle<double> clip;
        Font font;

        State& operator= (const State& o) {
            color = o.color;
            clip  = o.clip;
            font  = o.font;
            return *this;
        }
    };

    State state;
    std::vector<State> stack;

    bool _fill_dirty = false;

    void apply_pending_state() {
        if (_fill_dirty) {
            auto c = state.color;
            cairo_set_source_rgba (cr,
                                   c.fred(),
                                   c.fgreen(),
                                   c.fblue(),
                                   c.falpha());
            _fill_dirty = false;
        }
    }
};

class View : public lui::View {
public:
    View (Main& m, Widget& w)
        : lui::View (m, w) {
        set_backend ((uintptr_t) puglCairoBackend());
        set_view_hint (PUGL_DOUBLE_BUFFER, PUGL_FALSE);
        set_view_hint (PUGL_RESIZABLE, PUGL_TRUE);
        puglSetViewString ((PuglView*) c_obj(), PUGL_WINDOW_TITLE, w.name().c_str());
    }

    ~View() {}

    void expose (Bounds frame) override {
        auto cr = (cairo_t*) puglGetContext (_view);
        assert (cr != nullptr);

        if (true || ! _scale_set || _last_scale != scale_factor()) {
            _scale_set         = true;
            const auto scale_x = scale_factor();
            const auto scale_y = scale_x;
            _last_scale        = scale_x;
            if (auto s = cairo_get_target (cr))
                cairo_surface_set_device_scale (s, 1.0, 1.0);
            cairo_scale (cr, scale_x, scale_y);
        }

        cairo_save (cr);

#if 0
        // cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
        // cairo_rectangle (cr, 0, 0, bounds().width, bounds().height);
        // cairo_fill(cr);
        // cairo_restore (cr);
        // return;
#endif

#if __APPLE__ || 0
        // FIXME: needed on macOS until lvtk.Widget clipping problems
        // can be resolved.
        frame = bounds().at (0);
#endif

        if (_context->begin_frame (cr, frame)) {
            render (*_context);
            _context->end_frame();
        }

        cairo_restore (cr);
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
    PuglView* _view;
    std::unique_ptr<Context> _context;
    bool _scale_set { false };
    double _last_scale { 1.0 };
};
} // namespace cairo

std::unique_ptr<lui::View> Cairo::create_view (Main& c, Widget& w) {
    return std::make_unique<cairo::View> (c, w);
}

} // namespace lui
