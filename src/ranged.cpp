// SPDX-FileCopyrightText: 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <lui/ranged.hpp>

#include "detail/util.hpp"

namespace lui {
namespace detail {

class Ranged {
public:
    Ranged (lui::Ranged& o) : owner (o) {}

private:
    friend class lui::Ranged;
    lui::Ranged& owner;
    Range<double> range;
    double value = 0.0;
};

} // namespace detail

Ranged::Ranged() : impl (std::make_unique<detail::Ranged> (*this)) {}
Ranged::~Ranged() { impl.reset(); }

const Range<double>& Ranged::range() const noexcept { return impl->range; }
double Ranged::value() const noexcept { return impl->value; }

void Ranged::set_value (double value, Notify notify) {
    if (impl->value == value)
        return;

    impl->value = value;

    resized();
    repaint();

    if (notify == Notify::NONE)
        return;

    if (on_value_changed) {
        if (notify == Notify::SYNC)
            on_value_changed();
        else { /* trigger async callback somehow */
        }
    }
}

void Ranged::set_range (double min, double max) {
    if (min >= max)
        return;
    if (min != impl->range.min || max != impl->range.max) {
        impl->range.min = min;
        impl->range.max = max;
    }
}

} // namespace lui