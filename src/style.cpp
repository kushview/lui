// Copyright 2022 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <lui/style.hpp>

namespace lui {

Style::Style() {
    _weak_status.reset (this);
}

Style::~Style() {
    _weak_status.reset();
}

void Style::set_color (int ID, Color color) {
    _colors.insert (ColorItem { ID, color });
}

Color Style::find_color (int ID) const noexcept {
    if (_colors.empty())
        return {};
    auto it = _colors.find (ColorItem { ID, {} });
    if (it != _colors.end())
        return (*it).color;
    return {};
}

} // namespace lui
