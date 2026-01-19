// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

/** @defgroup widgets Widgets
    Part of the lvtk Widget's library.

    @{
*/

#pragma once

#include <lui/lui.h>

#include <lui/graphics.hpp>
#include <lui/input.hpp>
#include <lui/view.hpp>
#include <lui/weak_ref.hpp>

namespace lui {
namespace detail {
/** @private */
class Widget;
} // namespace detail

/** Base class for all Widgets and Windows.
    @ingroup widgets
    @headerfile lui/widget.hpp
*/
class LUI_API Widget {
public:
    /** Create a blank Widget */
    Widget();
    virtual ~Widget();

    /** Set the name of this widget */
    void set_name (const std::string& name);

    /** Get the name of this widget */
    const std::string& name() const noexcept;

    /** Returns the parent widget of this one. */
    Widget* parent() const noexcept;

    /** Add a child widget. */
    void add (Widget& widget);

    /** Add a child widget. */
    template <class Wgt>
    Wgt* add (Wgt* widget) {
        add_internal (widget);
        return widget;
    }

    /** Remove a child widget.
        @param widget The widget to remove
     */
    void remove (Widget* widget);

    /** Remove a child widget.
        @param widget The widget to remove.
     */
    void remove (Widget& widget);

    /** Returns true if this widget is visible. */
    [[nodiscard]] bool visible() const noexcept;

    /** Change this widget's visibility.
        @param visible True if it should be visible
     */
    void set_visible (bool visible);

    /** Show all child Widgets */
    void show_all();

    /** Request this widget be repainted. */
    void repaint();

    /** Returns true if this widget reports being opaque. */
    [[nodiscard]] bool opaque() const noexcept;

    /** Returns this widget's bounding box in parent coordinates.
        The position (x, y) is relative to the parent frame.
        Use this when you need to position or query the widget relative to its parent.
        
        @return The widget's bounds in parent coordinate space
    */
    [[nodiscard]] Bounds bounds() const noexcept;

    /** Returns this widget's local frame with origin at the top-left corner.
        This is equivalent to bounds().at(0) and represents the drawable area
        in the widget's own local coordinate space. The position (x, y) will
        typically be (0, 0), making this convenient for drawing operations.
        
        @return The widget's local frame with origin at (0, 0)
    */
    [[nodiscard]] Bounds frame() const noexcept;

    /** Returns the xy position of this widget.
        Same as calling widget.bounds().pos()
    */
    [[nodiscard]] Point<int> pos() const noexcept;

    /** Returns the x-coordinate of this widget parent space. */
    [[nodiscard]] int x() const noexcept;
    /** Returns the y-coordinate of this widget parent space. */
    [[nodiscard]] int y() const noexcept;
    /** Returns the width of this widget. */
    [[nodiscard]] int width() const noexcept;
    /** Returns the height of this widget. */
    [[nodiscard]] int height() const noexcept;

    /** Change this widget's bounds.
        
        @param x 
        @param y 
        @param width 
        @param height 
     */
    void set_bounds (int x, int y, int width, int height);

    /** Change this widget's bounds.
        
        @param b 
     */
    void set_bounds (Bounds b);

    /** Resize this widget.
     
        @param width
        @param height
    */
    void set_size (int width, int height);

    /** Returns true if the Widget can receive events at coordate xy. */
    virtual bool obstructed (int x, int y);

    /** True if this widget contains the other.
        
        @param widget The widget to test.
        @param deep If true search children recursively. default = false
    */
    bool contains (const Widget& widget, bool deep = false) const;

    /** Checks if a coordinate falls within this widget's local bounds.
        
        @param x The x-coordinate to test
        @param y The y-coordinate to test
        @return true if the coordinate is within the widget's local bounds
    */
    bool contains (int x, int y) const noexcept;

    /** Checks if a point falls within this widget's local bounds.
        
        @param coord The integer coordinate to test
        @return true if the coordinate is within the widget's local bounds
    */
    bool contains (Point<int> coord) const noexcept;

    /** Checks if a point falls within this widget's local bounds.
        
        @param coord The floating-point coordinate to test
        @return true if the coordinate is within the widget's local bounds
    */
    bool contains (Point<float> coord) const noexcept;

    /** Convert a coordinate from one widget's space to another.

        @param source The Widget to convert from
        @param coord A coordinate in the source widget's space
        @return The coordinate converted to this widget's space
     */
    Point<float> convert (const Widget* source, Point<float> coord) const;

    /** Converts a coordinate from local space to view space.
        
        @param coord The coordinate in local space to convert
        @return The coordinate in view space
     */
    Point<int> to_view_space (Point<int> coord);

    /** Converts a coordinate from local space to view space.
        
        @param coord The coordinate in local space to convert
        @return The coordinate in view space
     */
    Point<float> to_view_space (Point<float> coord);

    /** Called by the View to render this Widget.
        Invokes Widget::paint on this and all children recursively.
        
        @param g The graphics context to render into
     */
    void render (Graphics& g);

    /** Returns whether this widget is currently focused.
        
        @return true if the widget has focus, false otherwise
     */
    bool focused() const noexcept;

    /** Grab focus. */
    void grab_focus();

    /** Release focus. */
    void release_focus();

    /** True if this Widget owns it's View. i.e. is the top level
        widget under the OS window 
     */
    bool elevated() const noexcept;

    /** Returns the Widget underneath the given coordinate.
        @param pos The coordinate to check in local space
     */
    Widget* widget_at (Point<float> pos);

    /** Find the root widget in this tree */
    Widget* find_root() const noexcept;

    /** Find the view for this widget.
        locates the first elevated parent widget
        @returns View
     */
    View* find_view() const noexcept;

    /** Find the native handle for this Widget
        @returns uintptr_t
     */
    uintptr_t find_handle() const noexcept;

    /** Returns the current style used by this widget
        @returns Style The style used
     */
    Style& style();

protected:
    void set_opaque (bool opaque);

    virtual void resized() {}
    virtual void moved() {}

    virtual void paint (Graphics&) {}
    virtual void motion (const Event&) {}
    virtual void drag (const Event&) {}
    virtual void pressed (const Event&) {}
    virtual void released (const Event&) {}
    virtual void enter (const Event&) {}
    virtual void exit (const Event&) {}

    virtual bool key_down (const KeyEvent&) { return false; }
    virtual bool key_up (const KeyEvent&) { return false; }
    virtual bool text_entry (const TextEvent&) { return false; }

    virtual void children_changed() {}
    virtual void parent_structure_changed() {}
    virtual void parent_size_changed() {}
    virtual void child_size_changed (Widget* child) { lui::ignore (child); }

private:
    friend class detail::Widget;
    friend class Main;
    friend class detail::Main;
    friend class View;
    friend class detail::View;

    void add_internal (Widget*);

    std::unique_ptr<detail::Widget> impl;
    LUI_WEAK_REFABLE (Widget)
};

/** A Weak Reference to a Widget. */
using WidgetRef = WeakRef<Widget>;
} // namespace lui

/** @} */
