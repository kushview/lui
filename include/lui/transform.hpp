// Copyright 2024 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <cmath>

#include <lui/lui.h>
#include <lui/point.hpp>

namespace lui {

/** An affine transformation matrix.
  
    @ingroup graphics
    @headerfile lui/graphics.hpp
*/
class LUI_API Transform final {
public:
    Transform() = default;

    Transform (double mat00, double mat01, double mat02,
               double mat10, double mat11, double mat12)
        : m00 (mat00), m01 (mat01), m02 (mat02), m10 (mat10), m11 (mat11), m12 (mat12) {}

    bool operator== (const Transform& o) const noexcept {
        return m00 == o.m00
               && m01 == o.m01
               && m02 == o.m02
               && m10 == o.m10
               && m11 == o.m11
               && m12 == o.m12;
    }

    bool operator!= (const Transform& o) const noexcept {
        return ! operator== (o);
    }

    double m00 { 1.0 }; ///> The xx
    double m01 { 0.0 }; ///> The xy
    double m02 { 0.0 }; ///> The x0

    double m10 { 0.0 }; ///> The yx
    double m11 { 1.0 }; ///> The yy
    double m12 { 0.0 }; ///> The y0

    // clang-format off
    static Transform rotation (double angle) noexcept {
        auto angle_cos = std::cos (angle);
        auto angle_sin = std::sin (angle);
        return Transform { angle_cos, -angle_sin, 0,
                 angle_sin,  angle_cos, 0 };
    }

    static Transform translation (double dx, double dy) noexcept {
        return Transform { 1.0, 0.0, dx, 
                 0.0, 1.0, dy };
    }

    Transform translated (double dx, double dy) const noexcept {
        return { m00, m01, m02 + dx, 
                 m10, m11, m12 + dy };
    }

    Transform scaled (double scale_x, double scale_y) const noexcept {
        return { scale_x * m00, scale_x * m01, scale_x * m02, 
                 scale_y * m10, scale_y * m11, scale_y * m12 };
    }

    Transform scaled (double factor) const noexcept {
        return { factor * m00, factor * m01, factor * m02, 
                 factor * m10, factor * m11, factor * m12 };
    }

    /** Apply this transformation to a point.
        @param point The point to transform
        @return The transformed point
    */
    template <typename T>
    Point<T> map (const Point<T>& point) const noexcept {
        return {
            static_cast<T> (m00 * point.x + m01 * point.y + m02),
            static_cast<T> (m10 * point.x + m11 * point.y + m12)
        };
    }
    // clang-format on
};

} // namespace lui