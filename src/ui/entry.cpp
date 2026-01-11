// Copyright 2022 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <iostream>

#include <lui/entry.hpp>

#include "ui/detail/pugl.hpp"

namespace lui {
namespace detail {
class Entry {
public:
    Entry (lui::Entry& o) : owner (o) {}

    void paint (Graphics& g) {
        g.set_color (0x222222ff);
        g.fill_rect (owner.bounds().at (0));

        g.set_color (0xffffffff);
        font = font.with_height (15.f);
        g.set_font (font);
        g.draw_text (current_text,
                     owner.bounds().at (0).smaller (2).as<float>(),
                     Justify::MID_LEFT);
    }

    bool key_down (const KeyEvent& ev) {
        if (ev.key() == PUGL_KEY_BACKSPACE) {
            handle_backspace();
            owner.repaint();
            return true;
        } else if (ev.key() == PUGL_KEY_DELETE) {
            handle_delete();
            owner.repaint();
            return true;
        }
        return false;
    }

    bool text_entry (const TextEvent& ev) {
        auto osize = current_text.size();

        for (const auto c : ev.body) {
            if (c >= ' ' && c <= '~') {
                current_text += c;
            }
        }

        cursor = current_text.size();
        
        if (osize != current_text.size())
            owner.repaint();
        return true;
    }

    void handle_delete() {
        if (current_text.size() > cursor)
            current_text.erase (cursor, 1);
    }

    void handle_backspace() {
        if (cursor > 0 && !current_text.empty()) {
            --cursor;
            current_text.erase (cursor, 1);
        }
    }

private:
    friend class lui::Entry;
    lui::Entry& owner;
    std::string current_text;
    uint32_t cursor = 0;
    lui::Font font;
};

} // namespace detail

Entry::Entry() : impl (std::make_unique<detail::Entry> (*this)) {}
Entry::~Entry() { impl.reset(); }

void Entry::pressed (const Event& ev) { grab_focus(); }
void Entry::paint (Graphics& g) { impl->paint (g); }
bool Entry::key_down (const KeyEvent& ev) { return impl->key_down (ev); }
bool Entry::text_entry (const TextEvent& ev) { return impl->text_entry (ev); }

} // namespace lui
