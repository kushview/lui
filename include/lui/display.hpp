// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/lui.hpp>
#include <lui/rectangle.hpp>

namespace lui {

struct LUI_API Display {
    bool primary { false };
    double scale { 1.0 };
    double dpi { 96.0 };
};

} // namespace lui
