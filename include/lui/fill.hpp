// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/color.hpp>
#include <lui/image.hpp>
#include <lui/lui.h>

namespace lui {

/** A type of Fill used by DrawingContext
    i.e. fill Solid, Image, Gradient.
    @ingroup graphics
    @headerfile lui/fill.hpp
 */
class LUI_API Fill {
public:
    Fill() = default;
    void set_color (Color c) {
        _color = c;
    }

    Color color() const noexcept { return _color; }
    bool is_color() const noexcept { return true; }

    Fill (const Color& color) {
        set_color (color);
    }

    Fill (const Fill& o) { operator= (o); }
    Fill& operator= (const Fill& o) {
        _color = o._color;
        return *this;
    }

private:
    Color _color;
};

} // namespace lui
