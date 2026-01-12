// Copyright 2022 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include <iostream>

#include <lui/entry.hpp>

#include "detail/pugl.hpp"

namespace lui {
namespace detail {
class Entry {
public:
    Entry (lui::Entry& o) : owner (o) {}

    void paint (Graphics& g) {
        g.set_color (0xff000000);
        g.fill_rect (owner.bounds().at (0));

        auto bounds = owner.bounds().at (0).smaller (2).as<float>();
        
        g.set_color (0xffffffff);
        font = font.with_height (15.f);
        g.set_font (font);
        
        const auto fm = g.context().font_metrics();
        auto text_y = bounds.y + (bounds.height - static_cast<float> (fm.height)) * 0.5f;
        g.draw_text (current_text, Rectangle<float> { bounds.x, text_y, bounds.width, static_cast<float> (fm.height) }, Justify::TOP_LEFT);
        
        // Draw caret
        if (owner.focused()) {
            const auto tm = g.context().text_metrics (current_text.substr (0, cursor));
            
            auto caret_x = bounds.x + static_cast<float> (tm.width) + 2.f;
            auto caret_height = static_cast<float> (fm.height);
            auto caret_y = text_y;
            
            g.set_color (0xffffffff);
            g.fill_rect (Rectangle<float> { caret_x, caret_y, 2.f, caret_height });
        }
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

void Entry::pressed (const Event& ev) { 
    grab_focus(); 
    repaint();
}

void Entry::paint (Graphics& g) { impl->paint (g); }
bool Entry::key_down (const KeyEvent& ev) { return impl->key_down (ev); }
bool Entry::text_entry (const TextEvent& ev) { return impl->text_entry (ev); }

} // namespace lui
