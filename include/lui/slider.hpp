// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <functional>

#include <lui/lui.h>
#include <lui/notify.hpp>
#include <lui/range.hpp>
#include <lui/widget.hpp>

// clang-format off
namespace lui { namespace detail { class Ranged; } }
namespace lui { namespace detail { class Slider; } }
namespace lui { namespace detail { class Dial; } }
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
    double value() const noexcept;

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
    const Range<double>& range() const noexcept;

private:
    friend class detail::Ranged;
    std::unique_ptr<detail::Ranged> impl;
};

/** A typical Slider control.
    Can be styled as linear bar or thumb on a track

    @ingroup widgets
    @headerfile lui/slider.hpp
*/
class LUI_API Slider : public Ranged {
public:
    Slider();
    virtual ~Slider();

    /** The type of slider */
    enum Type : uint8_t {
        VERTICAL = 0,  ///< Vertical orientation with thumb/track
        HORIZONTAL,    ///< Horizontal orientation with thumb/track
        VERTICAL_BAR,  ///< Vertical orientation as solid bar
        HORIZONTAL_BAR ///< Horizontal orientation as solid bar
    };

    /** Change the type of slider */
    void set_type (Type type);
    /** Get the type of slider */
    Type type() const noexcept;
    /** Returns true if this has vertical orientation */
    bool vertical() const noexcept;

private:
    /** @private */
    bool obstructed (int x, int y) override { return true; }

    /** @private */
    void paint (Graphics& g) override;
    /** @private */
    void resized() override;
    /** @private */
    void drag (const Event&) override;
    /** @private */
    void pressed (const Event&) override;

    friend class detail::Slider;
    std::unique_ptr<detail::Slider> impl;
};

/** A type of dial. Like a Knob on a synth.
    @ingroup widgets
    @headerfile lui/slider.hpp
*/
class LUI_API Dial : public Ranged {
public:
    Dial();
    virtual ~Dial();

private:
    class Impl;
    std::unique_ptr<Impl> impl;

    /** @private */
    bool obstructed (int x, int y) override { return true; }
    /** @private */
    void paint (Graphics& g) override;
    /** @private */
    void resized() override;
    /** @private */
    void pressed (const Event&) override;
    /** @private */
    void drag (const Event&) override;
};

} // namespace lui
