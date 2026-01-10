// Copyright 2024 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/lui.h>
#include <lui/main.hpp>

namespace lui {

class Main;
class Widget;

/** The OpenGL graphics backend.
    Using this backend requires linking with NanoVG

    @ingroup widgets
    @ingroup graphics
    @headerfile lui/opengl.hpp 
*/
struct LUI_API OpenGL : public Backend {
    OpenGL() : Backend ("OpenGL") {}
    std::unique_ptr<View> create_view (Main& c, Widget& w) override;
};

} // namespace lui
