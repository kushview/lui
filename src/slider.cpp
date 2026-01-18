// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <lui/dial.hpp>
#include <lui/path.hpp>
#include <lui/ranged.hpp>
#include <lui/slider.hpp>

#include "detail/util.hpp"

namespace lui {
namespace detail {

class Slider {
public:
    using Type = lui::Slider::Type;

    Slider (lui::Slider& o) : owner (o) {
    }

    void paint (Graphics& g) {
        auto r = owner.bounds().at (0);
        owner.style().draw_slider (g, owner, r, divider);
    }

    void resized() {
        Range<double> pixel_range;
        pixel_range.min          = 0.0;
        const auto current_value = owner.value();
        float new_divider        = divider;
        if (owner.vertical()) {
            pixel_range.max = (double) owner.height();
            new_divider     = pixel_range.convert (owner.range(), current_value);
            new_divider     = pixel_range.max - new_divider;
        } else {
            pixel_range.max = (double) owner.width();
            new_divider     = pixel_range.convert (owner.range(), current_value);
        }

        if (new_divider != divider) {
            divider = new_divider;
            owner.repaint();
        }
    }

    void pressed (const Event& ev) {
        drag (ev);
    }

    void drag (const Event& ev) {
        Range<double> pixel_range;
        pixel_range.min  = 0.0;
        double new_value = owner.value();

        if (type == Type::VERTICAL || type == Type::VERTICAL_BAR) {
            divider         = detail::limit (ev.pos.y, 0.f, (float) owner.height());
            pixel_range.max = (double) owner.height();
            new_value       = owner.range().convert (pixel_range, divider);
            new_value       = owner.range().max - new_value;
        } else {
            divider         = detail::limit (ev.pos.x, 0.f, (float) owner.width());
            pixel_range.max = (double) owner.width();
            new_value       = owner.range().convert (pixel_range, divider);
        }

        owner.set_value (new_value, Notify::SYNC);
    }

    void set_type (Type t) {
        if (type == t)
            return;
        type = t;
        owner.resized();
        owner.repaint();
    }

private:
    friend class lui::Slider;
    lui::Slider& owner;
    float divider = 0.f;
    Type type { Type::VERTICAL_BAR };
};

} // namespace detail

Slider::Slider() : impl (std::make_unique<detail::Slider> (*this)) {}
Slider::~Slider() { impl.reset(); }

bool Slider::vertical() const noexcept {
    return impl->type == Slider::VERTICAL || impl->type == Slider::VERTICAL_BAR;
}

void Slider::set_type (Type type) { impl->set_type (type); }
Slider::Type Slider::type() const noexcept { return impl->type; }
void Slider::paint (Graphics& g) { impl->paint (g); }
void Slider::resized() { impl->resized(); }
void Slider::pressed (const Event& ev) { impl->pressed (ev); }
void Slider::drag (const Event& ev) { impl->drag (ev); }

} // namespace lui
