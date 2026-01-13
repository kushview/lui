// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

/** @mainpage LUI

    @section intro Introduction
    These documents describe some C++ classes that may be of use if you want
    to write LV2 plugins in C++. They implement most of the boilerplate code
    so you only have to write the parts that matter, and hide the low-level
    C API.

    <b>Library Versioning</b><br>
    The core library is header only, thus ABI stability is not an issue. The API
    will be stable between major version bumps, at which the pkg-config name
    would change to prevent plugins from building against an incompatible
    version.

    <b>ABI Versioning</b><br>
    The widgets and host libraries is not header only, thus ABI stability is
    a concern when adding new code. ABI version number will bump when compatibility
    breaks.

    @author Michael Fisher <mfisher@lvtk.org>
 */

#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <lui/lui.h>

namespace lui {
/** @defgroup alias Alias 
    Aliases to LV2 C-types 
 */

/** @defgroup utility Utility
    Utility classes and functions
 */

/** @defgroup wrapper Wrapper
    Slim wrappers around LV2 types.
 */

/** @defgroup graphics Graphics
    A graphics API
*/

/** @defgroup lvtk Core
    Core classes, types, and utilities.
    @{
 */

/* @} */
} // namespace lui
