// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/dial.hpp>
#include <lui/graphics.hpp>
#include <lui/path.hpp>
#include <lui/slider.hpp>
#include <lui/style.hpp>

namespace lui::detail {

class DefaultStyle : public Style {
public:
    DefaultStyle() {
        // orange
        set_color (ColorID::FOREGROUND, Color (0xFF, 0xA4, 0x00, 255));
        const auto fg = find_color (ColorID::FOREGROUND);

        set_color (ColorID::BUTTON_BASE, Color (0xff464646));
        set_color (ColorID::BUTTON_ON, fg.with_alpha (0.5f).darker (0.1f));
        set_color (ColorID::BUTTON_TEXT_OFF, Color (0xffeeeeee));
        set_color (ColorID::BUTTON_TEXT_ON, Color (0xffdddddd));

        set_color (ColorID::SLIDER_BASE, Color (0xff141414));
        set_color (ColorID::SLIDER_TRACK, Color (0xff090909));
        set_color (ColorID::SLIDER_THUMB, fg.darker (0.52f));

        set_color (ColorID::VIEW_BACKGROUND, Color (0xff000000));
    }

    ~DefaultStyle() {}

    void draw_button_shape (Graphics& g, lui::Button& w, bool highlight, bool down) override {
        auto bc = w.toggled() ? find_color (ColorID::BUTTON_ON) : find_color (ColorID::BUTTON_BASE);
        if (! w.toggled() && (highlight || down)) {
            if (! down)
                bc = bc.brighter (-0.015f);
            else
                bc = bc.brighter (-0.035f);
        }

        auto line_width = 1.2;
        auto r          = w.bounds().at (0).reduced (1);
        auto cs         = 4.f;

        g.set_color (bc);
        g.fill_rounded_rect (r, cs);

        g.set_color (w.toggled()
                         ? Color (0xFF, 0xA4, 0x00, 255).with_alpha (.70f)
                         : bc.darker (0.07f));
        g.context().set_line_width (line_width);
        g.draw_rounded_rect (r, cs);
    }

    void draw_button_text (Graphics& g, lui::TextButton& w, bool highlight, bool down) override {
        auto c = find_color (w.toggled() ? ColorID::BUTTON_TEXT_ON : ColorID::BUTTON_TEXT_OFF);
        if (! w.toggled() && (highlight || down))
            c = c.brighter (0.05f);
        else if (w.toggled()) {
            c = Color (0xff090909);
        }

        g.set_color (c);
        g.set_font (std::min (13.f, w.height() * 0.55f));
        auto r = w.bounds().at (0).as<float>();
        g.draw_text (w.text(), r, Justify::CENTERED);
    }

    void draw_slider (Graphics& g, lui::Slider& slider, Bounds bounds, float pos) override {
        auto r = bounds.as<float>();

        switch (slider.type()) {
            case lui::Slider::HORIZONTAL_BAR:
            case lui::Slider::VERTICAL_BAR: {
                g.set_color (find_color (ColorID::SLIDER_BASE));
                g.fill_rect (r);

                if (slider.vertical()) {
                    r.slice_top (pos);
                } else {
                    r = r.slice_left (pos);
                }

                g.set_color (find_color (ColorID::SLIDER_THUMB));
                g.fill_rect (r);
                break;
            }

            case lui::Slider::HORIZONTAL:
            case lui::Slider::VERTICAL: {
                g.save();
                draw_slider_background (g, slider, bounds, pos);
                g.restore();
                draw_slider_thumb (g, slider, bounds, pos);
                break;
            }
        }
    }

    void draw_slider_background (Graphics& g, lui::Slider& slider, Bounds bounds, float pos) override {
        (void) pos;

        int track_size = 4;

        if (slider.vertical()) {
            bounds.reduce ((slider.width() - track_size) / 2, 0);
        } else {
            bounds.reduce (0, (slider.height() - track_size) / 2);
        }

        g.set_color (find_color (ColorID::SLIDER_TRACK));
        g.fill_rect (bounds);
    }

    void draw_slider_thumb (Graphics& g, lui::Slider& slider, Bounds bounds, float pos) override {
        float thumb_size  = 16.f;
        float corner_size = 6.f;
        float x = 0.f, y = 0.f;

        Range<float> pixel_range (0.f, (float) (slider.vertical() ? bounds.height : bounds.width));
        float max_pixel = (float) (slider.vertical() ? bounds.height : bounds.width) - thumb_size;
        Range<float> thumb_range (4.f, max_pixel);
        pos = thumb_range.convert (pixel_range, pos);

        if (slider.vertical()) {
            x = ((float) slider.width() / 2.f) - (thumb_size / 2.f);
            y = pos;
        } else {
            x = pos;
            y = ((float) slider.height() / 2.f) - (thumb_size / 2.f);
        }

        g.set_color (find_color (ColorID::SLIDER_THUMB));
        g.fill_rounded_rect (x, y, thumb_size, thumb_size, corner_size);
    }

    void draw_dial (Graphics& g, Dial& dial, Bounds bounds) override {
        const auto r         = bounds.as<double>();
        const auto radius    = (float) std::min (r.width / 2, r.height / 2) - 2.0;
        const float center_x = r.x + (r.width * 0.5f);
        const float center_y = r.y + (r.height * 0.5f);
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

            if (true) // TODO(lui): enabled?
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
};

} // namespace lui::detail
