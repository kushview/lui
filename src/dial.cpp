// SPDX-FileCopyrightText: 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <algorithm>

#include <lui/dial.hpp>
#include <lui/path.hpp>

#include "detail/util.hpp"

namespace lui {

class Dial::Impl {
public:
    Impl()  = default;
    ~Impl() = default;
    Point<float> down_pos {};
    double down_value = 0.0;
};

Dial::Dial() : impl (new Impl()) {}
Dial::~Dial() {
    impl.reset();
}

void Dial::paint (Graphics& g) {
    style().draw_dial (g, *this, bounds().at (0));
}

void Dial::resized() {}

void Dial::pressed (const Event& ev) {
    impl->down_value = value();
    impl->down_pos   = ev.pos;
}

void Dial::drag (const Event& ev) {
    // This works but is hacky and needs to be better.

    auto& down_pos   = impl->down_pos;
    auto& down_value = impl->down_value;

    bool horizontal = true;
    auto delta      = horizontal ? ev.pos.x - down_pos.x
                                 : ev.pos.y - down_pos.y;
    double new_pos  = 0;
    new_pos         = delta / double (horizontal ? width() : height());
    new_pos *= std::max (1.0, range().diff() / 2.0);
    new_pos += down_value;
    new_pos = detail::limit (new_pos, range().min, range().max);

    if (value() != new_pos) {
        set_value (new_pos, Notify::SYNC);
    }
}
} // namespace lui
