// Copyright 2025 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

namespace lui {

/** A range of two values
    E.g. min and max with some conversion helpers
    @ingroup widgets
    @headerfile lui/slider.hpp
    @tparam Val the value type
 */
template <typename Val>
class Range {
public:
    /** Minimum value */
    Val min { Val() };
    /** Maximum value */
    Val max { Val() };

    /** Make an empty range */
    Range() = default;

    /** Make a range with min and max 
        @param minimum min Value
        @param maximum max Value
    */
    Range (Val minimum, Val maximum) : min (minimum), max (maximum) {}

    /** Returns true if min == max */
    constexpr bool empty() const noexcept { return min == max; }

    /** Returns the difference (max - min) */
    constexpr Val diff() const noexcept { return max - min; }

    /** Returns the proportion of val within min/max
        i.e. convert val to 0.0 - 1.0 
     */
    constexpr double ratio (Val val) const noexcept {
        return double (val - min) / double (max - min);
    }

    /** Convert a value from another range to fall within this one */
    constexpr Val convert (const Range<Val>& o, const Val& v) const noexcept {
        return (o.ratio (v) * static_cast<double> (diff())) + min;
    }

    bool operator== (const Range& o) const noexcept { return min == o.min && max == o.max; }
    bool operator!= (const Range& o) const noexcept { return min != o.min || max != o.max; }
};

} // namespace lui
