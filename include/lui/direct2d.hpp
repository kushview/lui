// Copyright 2026 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/lui.h>
#include <lui/main.hpp>

namespace lui {

class Main;
class Widget;

/** The Direct2D graphics backend.
    Using this backend requires Windows and links to Direct2D and DirectWrite.

    @ingroup widgets
    @ingroup graphics
    @headerfile lui/direct2d.hpp 
*/
struct LUI_API Direct2D : public Backend {
    Direct2D() : Backend ("Direct2D") {}
    std::unique_ptr<View> create_view (Main& c, Widget& w) override;
};

} // namespace lui
