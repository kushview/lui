// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/widget.hpp>

// clang-format off
namespace lui::detail { class Entry; }
// clang-format on

namespace lui {

/** Single line text entry widget 
    @ingroup widgets
    @headerfile lui/entry.hpp
*/
class LUI_API Entry : public Widget {
public:
    Entry();
    virtual ~Entry();

private:
    friend class detail::Entry;
    std::unique_ptr<detail::Entry> impl;

    /** @private */
    bool obstructed (int x, int y) override;
    /** @private */
    bool key_down (const KeyEvent& ev) override;
    /** @private */
    bool text_entry (const TextEvent& ev) override;
    /** @private */
    void paint (Graphics& g) override;
    /** @private */
    void pressed (const Event& ev) override;
};

} // namespace lui
