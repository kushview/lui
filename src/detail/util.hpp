// SPDX-FileCopyrightText: 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <algorithm>

namespace lui::detail {

template <typename T>
static inline T limit (const T& v, const T& a, const T& b) {
    return std::min (b, std::max (a, v));
}

} // namespace lui::detail
