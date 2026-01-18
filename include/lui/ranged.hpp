// SPDX-FileCopyrightText: Copyright (C) 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <functional>
#include <memory>

#include <lui/notify.hpp>
#include <lui/range.hpp>
#include <lui/widget.hpp>

// clang-format off
namespace lui::detail { class Ranged; }
// clang-format on

namespace lui {
/** A generic ranged Widget.
    Use  for base classes that need a min/max/value
    setup

    @ingroup widgets
    @headerfile lui/slider.hpp
    @see Slider,Dial
*/
class LUI_API Ranged : public Widget {
public:
    /** Create a new ranged */
    Ranged();

    virtual ~Ranged();

    /** Called when the value changes */
    std::function<void()> on_value_changed;

    /** @returns the current value */
    [[nodiscard]] double value() const noexcept;

    /** Set the current value
        @param value New value
        @param notify How to notify 
    */
    void set_value (double value, Notify notify);

    /** Set the min/max range
        @param min Min value
        @param max Max value
    */
    void set_range (double min, double max);

    /** Get the Range object used
        @returns the range
    */
    [[nodiscard]] const Range<double>& range() const noexcept;

private:
    friend class detail::Ranged;
    std::unique_ptr<detail::Ranged> impl;
};

} // namespace lui
