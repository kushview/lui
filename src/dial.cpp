// SPDX-FileCopyrightText: 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <algorithm>

#include <lui/dial.hpp>
#include <lui/path.hpp>

#include "detail/util.hpp"

namespace lui {
namespace detail {

// prototype dial draw function.  Eventually nees to move to style class.
inline static void draw_dial_proto (Graphics& g, lui::Dial& dial, Rectangle<double> r) {
    const auto radius    = (float) std::min (r.width / 2, r.height / 2) - 2.0;
    const float center_x = r.x + r.width * 0.5;
    const float center_y = r.y + r.height * 0.5;
    const float rx       = center_x - radius;
    const float ry       = center_y - radius;
    const float rw       = radius * 2.0f;

    auto da0 = -2.356194; // -45 degrees.
    auto da1 = 2.356194;  //  45 degrees.

    Range<double> range (0.0, 1.0);
    const auto sliderPos = (float) range.convert (dial.range(), dial.value());
    const auto anchorPos = 0.0;

    const float angle  = da0 + sliderPos * (da1 - da0);
    const float anchor = da0 + anchorPos * (da1 - da0);
    const float a1     = angle < anchor ? angle : anchor;
    const float a2     = angle < anchor ? anchor : angle;
    lui::ignore (a1, a2);

    const bool is_mouse_over = false;

    if (radius > 12.0f) {
        int line_size = (int) std::max (2.0, std::min (20.0, radius * 0.085));
        lui::ignore (line_size);
        float line_trim       = radius > 32.f ? -3.0f : -2.f;
        float line_offset     = radius > 32.f ? -4.f : -1.f;
        const float thickness = 0.82f;
        const float csf       = rw - (rw * thickness);
        {
            const float csf = rw - (rw * thickness);
            Path filled;
            filled.add_ellipse (Rectangle<float> (rx, ry, rw, rw).reduced (csf));
            g.set_color (Color (0xffffffff).darker (1.f));
            g.context().set_line_width (2.5);
            g.fill_path (filled);

            g.set_color (Color (0xff000000).brighter (0.17f));
            g.stroke_path (filled);
        }

        if (true) // TODO: enabled?
            g.set_color (Color (0xffcc0000).with_alpha (is_mouse_over ? 1.0f : 0.88f));
        else
            g.set_color (Color (0x80808080));

        {
            ScopedSave save (g.context());
            g.context().clear_path();
            g.context().transform (Transform::rotation (angle).translated (center_x, center_y));
            g.context().move_to (0.f, line_offset);
            g.context().line_to (0.f, -radius + csf + std::abs (line_trim));
            g.context().set_line_width (4);
            g.context().set_fill (Color (0xff000000));
            g.context().stroke();
        }
    } else {
    }
}
} // namespace detail

class Dial::Impl {
public:
    Impl()  = default;
    ~Impl() = default;
    Point<float> down_pos {};
    double down_value = 0.0;
};

Dial::Dial() : impl (new Impl()) {}
Dial::~Dial() {
    impl.reset();
}

void Dial::paint (Graphics& g) {
    detail::draw_dial_proto (g, *this, bounds().at (0).as<double>());
}

void Dial::resized() {}

void Dial::pressed (const Event& ev) {
    impl->down_value = value();
    impl->down_pos   = ev.pos;
}

void Dial::drag (const Event& ev) {
    // This works but is hacky and needs to be better.

    auto& down_pos   = impl->down_pos;
    auto& down_value = impl->down_value;

    bool horizontal = true;
    auto delta      = horizontal ? ev.pos.x - down_pos.x
                                 : ev.pos.y - down_pos.y;
    double new_pos  = 0;
    new_pos         = delta / double (horizontal ? width() : height());
    new_pos *= std::max (1.0, range().diff() / 2.0);
    new_pos += down_value;
    new_pos = detail::limit (new_pos, range().min, range().max);

    if (value() != new_pos) {
        set_value (new_pos, Notify::SYNC);
    }
}
} // namespace lui
