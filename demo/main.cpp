// Copyright 2019-2024 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <iostream>

#include <lui/main.hpp>
#include <lui/view.hpp>

#if LUI_DEMO_CAIRO
#    include <lui/cairo.hpp>
#    define LUI_DEMO_TITLE "LUI Cairo Demo"
#elif LUI_DEMO_VULKAN
#    include <lui/vulkan.hpp>
#    define LUI_DEMO_TITLE "LUI Vulkan Demo"
#elif LUI_DEMO_OPENGL
#    include <lui/opengl.hpp>
#    define LUI_DEMO_TITLE "LUI OpenGL Demo"
#endif

#include "demo.hpp"

namespace lui {
namespace demo {

template <class Wgt>
static int run (lui::Main& context) {
    try {
        auto content = std::make_unique<Wgt>();
        content->set_name (LUI_DEMO_TITLE);
        if (auto view = context.elevate (*content, lui::View::RESIZABLE, 0)) {
            view->set_position ((1920 / 2) - (view->bounds().width / 2),
                                (1080 / 2) - (view->bounds().height / 2));
        }

        while (true) {
            context.loop (1.0 / 60.0);
            if (! context.running())
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "[demo] fatal error in main loop" << std::endl
                  << "[demo] " << e.what() << std::endl;
        context.set_exit_code (-1);
    }

    std::clog << "[demo] exiting with code: " << context.exit_code() << std::endl;
    return context.exit_code();
}

} // namespace demo
} // namespace lui

#ifdef _WIN32
#    include <windows.h>
int WinMain (HINSTANCE hInstance,
             HINSTANCE hPrevInstance,
             LPSTR lpCmdLine,
             int nShowCmd) {
    (hInstance, hPrevInstance, lpCmdLine, nShowCmd);

    struct ClogBuf : public std::stringbuf {
        ~ClogBuf() { sync(); }
        int sync() {
            ::OutputDebugStringA (str().c_str());
            str (std::string()); // Clear the string buffer
            return 0;
        }
    } dbgbuf;

    auto clogbuf = std::clog.rdbuf (&dbgbuf);

#    if LUI_DEMO_CAIRO
    lui::Main context (lui::Mode::PROGRAM, std::make_unique<lui::Cairo>());
#    elif LUI_DEMO_VULKAN
    lui::Main context (lui::Mode::PROGRAM, std::make_unique<lui::Vulkan>());
#    else
    lui::Main context (lui::Mode::PROGRAM, std::make_unique<lui::OpenGL>());
#    endif

    auto ret = lui::demo::run<lui::demo::Content> (context);

    std::clog.rdbuf (clogbuf);
    return ret;
}
#else
int main (int argc, char** argv) {
#    if LUI_DEMO_CAIRO
    lui::Main context (lui::Mode::PROGRAM, std::make_unique<lui::Cairo>());
#    elif LUI_DEMO_VULKAN
    lui::Main context (lui::Mode::PROGRAM, std::make_unique<lui::Vulkan>());
#    else
    lui::Main context (lui::Mode::PROGRAM, std::make_unique<lui::OpenGL>());
#    endif
    return lui::demo::run<lui::demo::Content> (context);
}
#endif
