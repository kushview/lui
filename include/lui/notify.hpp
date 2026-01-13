// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <cstdint>

namespace lui {

/** A type of notification
    @ingroup widgets
    @headerfile lui/slider.hpp
 */
enum class Notify : uint32_t {
    NONE = 0, ///< Don't send any notification
    SYNC,     ///< Send notification sync
    ASYNC     ///< Send notification async
};

} // namespace lui
