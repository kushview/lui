// SPDX-FileCopyrightText: 2022 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

// Pugl amalgamation - compile pugl by including source files directly

#include "pugl/src/common.c"
#include "pugl/src/internal.c"

#if defined(__APPLE__)
    // macOS platform code is in pugl.mm
#elif defined(_WIN32) || defined(_WIN64)
    #include "pugl/src/win.c"
    // #include "pugl/src/win_cairo.c"    // Requires cairo
    #include "pugl/src/win_gl.c"
    #include "pugl/src/win_stub.c"
    // #include "pugl/src/win_vulkan.c"   // Requires vulkan
#else  // Linux/X11
    #include "pugl/src/x11.c"
    // #include "pugl/src/x11_cairo.c"    // Requires cairo
    #include "pugl/src/x11_gl.c"
    #include "pugl/src/x11_stub.c"
    // #include "pugl/src/x11_vulkan.c"   // Requires vulkan
#endif
