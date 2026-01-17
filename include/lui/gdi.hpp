// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/lui.h>
#include <lui/main.hpp>

namespace lui {

class Main;
class Widget;

/** The Windows GDI graphics backend.
    Using this backend requires Windows and links to GDI.

    @ingroup widgets
    @ingroup graphics
    @headerfile lui/gdi.hpp 
*/
struct LUI_API GDI : public Backend {
    GDI() : Backend ("GDI") {}
    std::unique_ptr<View> create_view (Main& c, Widget& w) override;
};

} // namespace lui
