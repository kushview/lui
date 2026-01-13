// Copyright 2024 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/lui.h>
#include <lui/main.hpp>

namespace lui {

class Main;
class Widget;

/** The CoreGraphics graphics backend.
    Using this backend requires macOS and links to CoreGraphics framework.

    @ingroup widgets
    @ingroup graphics
    @headerfile lui/core_graphics.hpp 
*/
struct LUI_API CoreGraphics : public Backend {
    CoreGraphics() : Backend ("CoreGraphics") {}
    std::unique_ptr<View> create_view (Main& c, Widget& w) override;
};

} // namespace lui
