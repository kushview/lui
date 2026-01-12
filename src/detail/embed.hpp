// Copyright 2022 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/embed.hpp>
#include <lui/rectangle.hpp>

#include <iostream>

namespace lui {
namespace detail {

class Embed {
public:
    class Proxy : public lui::Widget {
    public:
        Proxy() {}
        ~Proxy() {}
    };

    virtual ~Embed() = default;

    void resized() {
        if (! proxy)
            return;

        if (auto pv = proxy->find_view()) {
            Bounds r = owner.bounds();
            auto pos = owner.to_view_space (r.pos());
            r.x      = pos.x;
            r.y      = pos.y;

            if (! r.empty()) {
                pv->set_bounds (r);
                proxy->set_size (r.width, r.height);
            }
        }
    }

    virtual void create_proxy_view() {
        if (proxy != nullptr)
            return;

        if (auto owner_view = owner.find_view()) {
            create_proxy();
            if (proxy) {
                owner_view->elevate (*proxy, 0);
                if (auto pv = proxy->find_view()) {
                    proxy->set_size (240, 360);
                    proxy->set_visible (true);
                    pv->set_visible (true);
                }
            }
        } else {
            // std::clog << "[embed] window: owner didn't get parent view\n";
        }
    }

    inline lui::View* proxy_view() const noexcept {
        return proxy != nullptr ? proxy->find_view() : nullptr;
    }

    inline void close_proxy() {
        if (proxy) {
            proxy->set_visible (false);
            proxy.reset();
        }
    }

    inline void create_proxy() {
        close_proxy();
        proxy = std::make_unique<Proxy>();
        proxy->set_size (1, 1);
    }

protected:
    Embed (lui::Embed& o) : owner (o) {}
    lui::Embed& owner;
    std::unique_ptr<Proxy> proxy;

private:
    friend class lui::Embed;
    /** @internal Platform specific factory function. */
    static std::unique_ptr<detail::Embed> create (lui::Embed&);
};

} // namespace detail
} // namespace lui
