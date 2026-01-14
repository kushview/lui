// Copyright 2024 Kushview, LLC
// SPDX-License-Identifier: ISC

#include <lui/cairo.hpp>
#include <lui/lui.hpp>
#include <lui/string.hpp>

#include "./bindings.h"
#include "./proxy.hpp"
#include "./sol/sol.hpp"

namespace sol {
#if 0
/** Support lui::WeakRef in sol2. Does not work yet  */
template <typename T>
struct unique_usertype_traits<lui::WeakRef<T>>
{
    using type = lui::WeakRef<T>;
    using actual_type = T;
    static const bool value = true;
    static bool is_null (const actual_type& ptr) { return ptr == nullptr; }
    static type* get (const actual_type& ptr) { return ptr.lock(); }
};
#endif
} // namespace sol

using namespace sol;
using namespace lui;

namespace lua {

// Lua Binding Helpers
// This was thrown together, but could be tidy'd up and added
// to the public headers. Coders could then write custom widgets
// and have a helper library to bind them to Lua.  Class inheritence
// is working but there's alot of manual steps involved.

/** Removes a field from the table then clears it.
    
    @param tbl Input table
    @param field The field to remove. Lua type MUST be a table
*/
inline static table remove_and_clear (table tbl, const char* field) {
    auto F = tbl.get<table> (field);
    tbl.clear();
    return F;
}

template <class Self>
inline static std::string tostring (Self& self, const char* NS, const char* name) {
    std::stringstream stream;
    stream << NS << "." << name << ": 0x" << std::hex << (intptr_t) &self;
    return stream.str();
}

// deps required by Widget, excluding itself
//
static inline void require_widget_deps (lua_State* L) {
    state_view view (L);
    view.script (R"(
        require ('lui.Point')
        require ('lui.Bounds')
        require ('lui.Graphics')
    )");
}

//====== Class binding helpers
/** Bind adding tostring meta method */
template <typename Obj, typename... Args>
inline static auto bind (lua_State* L, const char* NS, const char* name, Args&&... args) {
    state_view lua (L);
    table M = lua.create_table();
    M.new_usertype<Obj> (
        name, no_constructor, meta_method::to_string, [&] (Obj& self) {
            return lua::tostring (self, NS, name);
        },
        std::forward<Args> (args)...);
    return remove_and_clear (M, name);
}

/** binds without adding tostring meta method */
template <typename Obj, typename... Args>
inline static auto bind (lua_State* L, const char* name, Args&&... args) {
    state_view lua (L);
    table M = lua.create_table();
    M.new_usertype<Obj> (
        name, no_constructor, std::forward<Args> (args)...);
    return remove_and_clear (M, name);
}

inline static auto script (lua_State* L, const char* body) {
    return state_view (L).script (body);
}

/** Subclasses of Widget in C++ should register with this variation
 *  then call require_widget_deps before pushing to the stack
 */
template <typename Wgt, typename... Args>
inline static table
    bind_widget (lua_State* L, const char* name, Args&&... args) {
    // clang-format off
    auto M = lua::bind<Wgt> (L, "lui", name,
        /// Initialize the widget.
        // Override this to customize your widget.
        // @function Widget.init
        "init",
        Wgt::init,

        /// Widget visibility (bool).
        // Shows or hides this Widget
        // @field Widget.visible
        "visible",
        property (&Wgt::visible, &Wgt::set_visible),

        /// Methods.
        // @section methods

        /// Returns the bounding box.
        // @function Widget:bounds
        "bounds", property(&Wgt::proxy_get_bounds, &Wgt::proxy_set_bounds),
        // "bounds", &Wgt::bounds,

        "obstructed",
        &Wgt::obstructed,

        /// Change the bounding box.
        // The coords returned is relative to the top/left of the widget's parent.
        // @function Widget:setbounds
        // @tparam mixed New bounds as a kv.Bounds or table
        // @usage
        // -- Can also set a table. e.g.
        // widget:setbounds ({
        //     x      = 0,
        //     y      = 0,
        //     width  = 100,
        //     height = 200
        // })
        "set_bounds", 
        overload (
            [] (Wgt& self, double x, double y, double w, double h) { 
                self.set_bounds (lui::Rectangle<double> { x, y, w, h }.as<int>()); 
            }, [](Wgt& self, const object& obj) { 
                lui::proxy::widget_set_bounds (self, obj);
            }),
        "set_size", &Wgt::set_size,
        std::forward<Args> (args)...);
    // clang-format on
    state_view view (L);

    ////
    // begin C++ <=> Lua class inheritance setup
    ////

    // turn of "operator[] so attributes defined in lua
    // can be resolved
    auto M_mt          = M[metatable_key];
    M_mt["__newindex"] = lua_nil;

    // Called when widget is instantiated by lui.class
    M_mt["__newuserdata"] = [L]() {
        state_view view (L);
        return std::make_unique<Wgt> (view.create_table());
    };

    // attribute lookup keys used by lua-side proxy
    M_mt[lui::proxy::props_key] = view.create_table().add (
        "visible", "bounds");

    // class method symbols used by lua-side proxy
    M_mt[lui::proxy::methods_key] = view.create_table().add (
        "add",
        "set_bounds",
        "set_size");

    require_widget_deps (L);
    view.script (R"(
        require ('lui.Graphics')
    )");
    return M;
}

} // namespace lua

// clang-format off
//=============================================================================
#include <lui/context.hpp>
LUI_LUALIB
int luaopen_lui_Context (lua_State* L) {
    // This'll need re-done in plain lua if it ever
    // becomes a performance hit in realtime/dsp
    // contexts.
    auto M = lua::bind<lui::Context> (L, "lui", "Context");

    lua::script (L, R"(
        require ('lui.Graphics')
    )");
    
    stack::push (L, M);
    return 1;
}

//=============================================================================
#include <lui/graphics.hpp>
LUI_LUALIB
int luaopen_lui_Point (lua_State* L) {
    using Point = Point<float>;
    using T = float;
    auto M = lua::bind<Point> (L, "lui", "Point",
        "x",        &Point::x,
        "y",        &Point::y,
        "new", factories (
            []() { return Point(); },
            [](T x, T y) { return Point{ x, y }; }
        ),
        meta_method::to_string, [](const Point& self) {
            auto s = lua::tostring (self, "lui", "Point");
            s += std::string(": ") + self.str();
            return s;
        }
    );

    // doesn't depend on anything, no requires
    stack::push (L, M);
    return 1;
}

LUI_LUALIB
int luaopen_lui_Bounds (lua_State* L) {
    using R = Bounds;
    using T = int;
    auto M = lua::bind<Bounds> (L, "lui", "Bounds",
        "x",        &Bounds::x,
        "y",        &Bounds::y,
        "width",    &Bounds::width,
        "height",   &Bounds::height,
        "at",    [](Bounds& self, lua_Integer x, lua_Integer y) -> R {
            return self.at (x, y);
        },

        "new", factories (
            []() { return R(); },
            [](T x, T y, T w, T h) { return R{ x, y, w, h }; },
            [](T w, T h) { return R { T(), T(), w, h }; }
        ),

        meta_method::to_string, [](const Bounds& self) {
            auto s = lua::tostring (self, "lui", "Bounds");
            s += std::string(": ") + self.str();
            return s;
        }
    );

    state_view(L).script (R"(
        require ('lui.Point')
    )");
    stack::push (L, M);
    return 1;
}

LUI_LUALIB
int luaopen_lui_Graphics (lua_State* L) {
    static constexpr const char* ns = "lui";
    static constexpr const char* name = "Graphics";
    auto M = lua::bind<lui::Graphics> (L, ns, name,
        "last_clip", &Graphics::last_clip,
        "translate", &Graphics::translate,
        "save", &Graphics::save,
        "restore", &Graphics::restore,
        "set_color", [](Graphics& self, lua_Integer c) {
            self.set_color (lui::Color (static_cast<uint32_t> (c)));
            self.set_color (lui::Color (0xff0000ff));
        },
        "fill_rect", [](Graphics& self, Bounds r) {
                self.fill_rect (r);
            }
    );

    stack::push (L, M);
    return 1;
}

LUI_LUALIB
int luaopen_lui_DrawingContext (lua_State* L) {
    static constexpr const char* ns = "lui";
    static constexpr const char* name = "DrawingContext";
    auto M = lua::bind<lui::DrawingContext> (L, ns, name,
        "dummy", []() {}
    );

    stack::push (L, M);
    return 1;
}

//=============================================================================
#include <lui/input.hpp>
LUI_LUALIB
int luaopen_lui_Event (lua_State* L) {
    auto T = lua::bind<lui::Event> (L, "lui", "Event",
        "source", readonly_property (&Event::source),
        "pos", readonly_property (&Event::pos),
        "x", readonly_property (&Event::x),
        "y", readonly_property (&Event::y)
    );

    stack::push (L, T);
    return 1;
}

//=============================================================================
#include <lui/main.hpp>
LUI_LUALIB
int luaopen_lui_Main (lua_State* L) {
    auto T = lua::bind<lui::Main> (L, "lui", "Main",
        "loop",     &Main::loop,
        "running",  &Main::running,
        "elevate", [](lui::Main& self, object tbl) {
            // std::clog << "elevate: " << type_name (L, tbl.get_type()) << std::endl;
            // TODO: deeply inspect that this really is a widget
            // if not rais a lua error
            if (tbl.is<table>()) {
                if (auto proxy = lui::proxy::userdata<lui::proxy::Widget> ((table)tbl))
                    self.elevate (*proxy, 0, 0);
            } else {
                // std::clog << "not a table: \n";
            }
        },
        
        "mode", [](lui::Main& self) -> std::string {
            switch (self.mode()) {
                case lui::Mode::PROGRAM: return "program"; break;
                case lui::Mode::MODULE: return "module"; break;
            };
            return "undefined";
        },

        "quit", &Main::quit,
        
        "exit_code", &Main::exit_code,
        "new", factories ([]() {
            auto obj = std::make_unique<lui::Main>(
                lui::Mode::PROGRAM,
                std::make_unique<lui::Cairo>()
            );

            return obj;
        })
    );

    lua::script (L, R"(
        require ('lui.Graphics')
        require ('lui.Widget')
        require ('lui.View')
        require ('lui.Event')
    )");

    stack::push (L, T);
    return 1;
}

//=============================================================================
#include <lui/view.hpp>
LUI_LUALIB
int luaopen_lui_View (lua_State* L) {
    auto T = lua::bind<lui::View> (L, "lui", "View",
        "visible",  property (&View::visible, &View::set_visible),
        "bounds",       &View::bounds,
        "scale_factor", &View::scale_factor,
        "set_size",     &View::set_size
    );
    stack::push (L, T);
    return 1;
}

//=============================================================================
#include <lui/widget.hpp>
LUI_LUALIB
int luaopen_lui_Widget (lua_State* L) {
    using W = lui::proxy::Widget;
    auto M = lua::bind_widget<W> (L, "Widget", 
        "add",      &W::add
    );

    lua::require_widget_deps (L);
    stack::push (L, M);
    return 1;
}

// main module
LUI_LUALIB
int luaopen_lvtk (lua_State* L) {
    // lock and load
    state_view view (L);
    stack::push (L, view.script (R"(
    local M = {
        bytes       = require ('lui.bytes'),
        import      = require ('lui.import'),
        object      = require ('lui.class'),
        midi        = require ('lui.midi'),
        round       = require ('lui.round'),
        
        Point       = require ('lui.Point'),
        Bounds      = require ('lui.Bounds'),
        Surface     = require ('lui.Surface'),
        Graphics    = require ('lui.Graphics'),
        Event       = require ('lui.Event'),
        Main        = require ('lui.Main'),
        View        = require ('lui.View'),
        Widget      = require ('lui.Widget'),
        World       = require ('lui.World'),
        Symbols     = require ('lui.Symbols')
    }
    return M
    )"));

    return 1;
}

// clang-format on
