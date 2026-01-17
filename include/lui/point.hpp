// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <algorithm>
#include <type_traits>

#include <lui/string.hpp>

namespace lui {

/** A point. x y coordinate.
    @ingroup graphics
    @headerfile lui/point.hpp
*/
template <typename Val>
struct Point {
    static_assert (std::is_integral_v<Val> || std::is_floating_point_v<Val>,
                   "Point value type must be integral or floating point");

    /** X coordinate */
    Val x {};
    /** Y coordinate */
    Val y {};

    Point() = default;

    template <typename TX, typename TY,
              typename = std::enable_if_t<
                  (std::is_integral_v<TX> || std::is_floating_point_v<TX>) && (std::is_integral_v<TY> || std::is_floating_point_v<TY>)>>
    Point (TX x_val, TY y_val) noexcept
        : x (static_cast<Val> (x_val)),
          y (static_cast<Val> (y_val)) {}

    /** Convert this point to another value type.
        i.e. int to float
    */
    template <typename T>
    Point<T> as() const noexcept {
        return {
            static_cast<T> (x),
            static_cast<T> (y)
        };
    }

    Point operator- (const Point& o) const noexcept {
        Point copy (*this);
        copy -= o;
        return copy;
    }

    Point& operator-= (const Point& o) noexcept {
        x -= o.x;
        y -= o.y;
        return *this;
    }

    Point operator+ (const Point& o) const noexcept {
        Point copy (*this);
        copy += o;
        return copy;
    }

    Point& operator+= (const Point& o) noexcept {
        x += o.x;
        y += o.y;
        return *this;
    }

    template <typename OT>
    Point operator* (OT m) const noexcept {
        using T = typename std::common_type<Val, OT>::type;
        return { (T) ((T) x * (T) m),
                 (T) ((T) y * (T) m) };
    }

    template <typename OT>
    Point operator/ (OT d) const noexcept {
        using T = typename std::common_type<Val, OT>::type;
        return { (T) ((T) x / (T) d),
                 (T) ((T) y / (T) d) };
    }

    /** Convert to a String.
        Format of output = "${x} ${y}"
    */
    std::string str() const noexcept {
        String st;
        st << x << " " << y;
        return st;
    }
};

} // namespace lui
