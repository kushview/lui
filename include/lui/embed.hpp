// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/lui.h>
#include <lui/widget.hpp>

namespace lui {
namespace detail {
/** @private */
class Embed;
} // namespace detail

/** Embed native views in another Widget. 
    @ingroup widgets
    @headerfile lui/embed.hpp
 */
class LUI_API Embed final : public Widget {
public:
    /** Initialize an embeddable widget */
    Embed();
    ~Embed();

    /** Returns the view of the embedded object
        @returns the host view of the proxy.
     */
    ViewRef proxy_view() const noexcept;

protected:
    /** @internal */
    void paint (Graphics& g) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void children_changed() override;
    /** @internal */
    void parent_structure_changed() override;

private:
    std::unique_ptr<detail::Embed> impl;
    LUI_DISABLE_COPY (Embed)
};

} // namespace lui
