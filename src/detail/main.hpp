// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <atomic>

#define PUGL_DISABLE_DEPRECATED
#include <pugl/pugl.h>

#include <lui/button.hpp>
#include <lui/main.hpp>
#include <lui/slider.hpp>
#include <lui/view.hpp>

#include "detail/view.hpp"

namespace lui {
namespace detail {

static inline PuglWorldType world_type (Mode mode) {
    if (mode == Mode::PROGRAM)
        return PUGL_PROGRAM;
    else if (mode == Mode::MODULE)
        return PUGL_MODULE;
    return PUGL_MODULE;
}

static inline PuglWorldFlags world_flags() {
    return 0;
}

class Main {
public:
    Main (lui::Main& o, const Mode m, std::unique_ptr<lui::Backend> b);
    std::unique_ptr<lui::View> create_view (lui::Widget& widget, ViewFlags flags, uintptr_t parent);

    bool running() const noexcept {
        return first_loop_called
               && last_update_status == PUGL_SUCCESS
               && ! quit_flag;
    }

    bool loop (double timeout);

private:
    friend class lui::Main;
    friend class lui::View;
    friend class detail::View;

    lui::Main& owner;
    const Mode mode;
    PuglWorld* world { nullptr };
    std::unique_ptr<lui::Backend> backend;
    std::vector<lui::View*> views;
    std::unique_ptr<lui::Style> style;
    bool quit_flag { false };
    std::atomic<int> exit_code { 0 };

    bool first_loop_called { false };
    PuglStatus last_update_status = PUGL_UNKNOWN_ERROR;
};

} // namespace detail
} // namespace lui
