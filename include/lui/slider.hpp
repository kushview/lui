// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/ranged.hpp>

// clang-format off
namespace lui::detail { class Slider; }
// clang-format on

namespace lui {

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
    [[nodiscard]] Type type() const noexcept;
    /** Returns true if this has vertical orientation */
    [[nodiscard]] bool vertical() const noexcept;

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

} // namespace lui
