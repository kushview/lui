// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <lui/color.hpp>
#include <lui/fill.hpp>
#include <lui/fitment.hpp>
#include <lui/font.hpp>
#include <lui/image.hpp>
#include <lui/justify.hpp>
#include <lui/lui.h>
#include <lui/rectangle.hpp>
#include <lui/transform.hpp>

namespace lui {

class Path;

/** A scoped save & restore helper. The constructor calls `save()`, the 
    destructor calls `restore()`
*/
template <class Save>
struct ScopedSave {
    using save_type = Save;
    ScopedSave()    = delete;

    /** Save without arguments */
    ScopedSave (Save& s) noexcept : _ref (s) { _ref.save(); }

    /** Save with arguments */
    template <typename... Args>
    ScopedSave (Save& s, Args&&... args)
        : _ref (s) { _ref.save (std::forward<Args> (args)...); }
    ~ScopedSave() noexcept { _ref.restore(); }

private:
    Save& _ref;
    LUI_DISABLE_COPY (ScopedSave)
    LUI_DISABLE_MOVE (ScopedSave)
};

struct FontMetrics {
    double ascent { 0.0 };
    double descent { 0.0 };
    double height { 0.0 };
    double x_stride_max { 0.0 };
    double y_stride_max { 0.0 };
};

struct TextMetrics {
    double width { 0.0 };
    double height { 0.0 };
    double x_offset { 0.0 };
    double y_offset { 0.0 };
    double x_stride { 0.0 };
    double y_stride { 0.0 };
};

/** Lower level graphics context.
    @ingroup graphics
    @headerfile lui/graphics.hpp
*/
class LUI_API DrawingContext {
public:
    DrawingContext()          = default;
    virtual ~DrawingContext() = default;

    // clang-format off
    virtual double device_scale() const noexcept =0;
    
    /** Save the current state. */
    virtual void save() =0;

    /** Restore last state. */
    virtual void restore() =0;

    /** Set the line width. */
    virtual void set_line_width (double width) =0;

    /** Clears the current path. After calling there will be no path and no 
        current position.
     */
    virtual void clear_path() =0;

    /** Begin a new sub-path. After calling the current point will be (x, y). */
    virtual void move_to (double x, double y) =0;
    
    /** Draw a line */
    virtual void line_to (double x1, double y1) =0;
    
    /** Draw a quadratic curve */
    virtual void quad_to (double x1, double y1, double x2, double y2) =0;
    
    /** Adds a cubic Bézier curve to the path from the current point to 
        position (x3 , y3) in user-space coordinates. The (x1, y1) and 
        (x2, y2) are used as the control points. After returning current point 
        will be (x3, y3).

        If there is no current point before the call to cubic_to() this 
        function will behave as if preceded by calling move_to (x1, y1).
    */
    virtual void cubic_to (double x1, double y1, double x2, double y2, double x3, double y3) =0;
    
    /** Close the current path */
    virtual void close_path() =0;

    /** Fill the current path with the currrent settings */
    virtual void fill() =0;
    
    /** Stroke the current path with current settings */
    virtual void stroke() =0;

    /** Translate the origin */
    virtual void translate (double dx, double dy) =0;

    /** Apply transformation matrix */
    virtual void transform (const Transform& mat) =0;
    
    virtual void clip (const Rectangle<int>& r) =0;
    virtual void exclude_clip (const Rectangle<int>& r) =0;
    virtual Rectangle<int> last_clip() const =0;

    /** Get the current font.
        Return the last font set with set_font
        @returns Font
    */
    virtual Font font() const noexcept =0;

    /** Set the current font.
        @param font Font to use for text ops.
    */
    virtual void set_font (const Font& font) =0;

    /** Set the current fill type.
        Subclass should save the fill and use it for stroke/fill operations
        @param fill The new fill type to use
    */
    virtual void set_fill (const Fill& fill) =0;
    
    virtual void fill_rect (const Rectangle<double>& r) =0;
    // clang-format on

    /** Returns the font metrics for the currently selected font. */
    virtual FontMetrics font_metrics() const noexcept = 0;

    /** Returns the text metrics for the currently selected font. */
    virtual TextMetrics text_metrics (std::string_view text) const noexcept = 0;

    /** Draw some text.
        
        Implementations should draw the text with the current font at x/y.
        Justify applies the the point, not the space being drawn in to.

        @param text The text to draw.
     */
    virtual bool show_text (const std::string_view text) {
        lui::ignore (text);
        return false;
    }

    virtual void draw_image (Image image, Transform transform) {
        lui::ignore (image, transform);
    }
};

/** Higher level graphics context.
    API is subject to change dramatically at any given time
    until we approach an alpha status.

    @ingroup graphics
    @headerfile lui/graphics.hpp
 */
/**
 * @class Graphics
 * @brief A graphics rendering context for drawing shapes, text, and images.
 * 
 * The Graphics class provides a high-level API for 2D drawing operations
 * including rectangles, paths, text, and images. It manages drawing state
 * through a stack-based save/restore mechanism and supports clipping regions.
 * 
 * @note This class cannot be instantiated without a DrawingContext.
 * @note Instances are non-copyable.
 */
class LUI_API Graphics final {
public:
    Graphics (DrawingContext& d);
    Graphics()  = delete;
    ~Graphics() = default;

    /** Returns the underlying DrawingContext used by this Graphics instance.
        
        @return A reference to the DrawingContext
    */
    DrawingContext& context();

    /** Saves the current graphics state (font, color, line width, etc.)
        to the internal stack. Pairs with restore().
    */
    void save();

    /** Restores the graphics state from the internal stack.
        Restores all settings saved by the most recent save() call.
    */
    void restore();

    /** Translates the origin by the specified delta from the current origin.
        
        @param delta The offset to move the origin by (x and y components)
    */
    void translate (Point<int> delta);

    /** Sets the clip bounds to restrict drawing to a specific region.
        
        @param c The bounds to set as the clip region
    */
    void clip (Bounds c);

    /** Excludes a rectangle from the current clip region.
        
        @param c The region to exclude from clipping
    */
    void exclude_clip (Bounds c);

    /** Returns the current clip bounds that were last set with clip().
        
        @return The current clip bounds
    */
    [[nodiscard]] Bounds last_clip() const noexcept;

    /** Returns whether the current clip region is empty.
        
        @return true if the clip region is empty, false otherwise
    */
    [[nodiscard]] bool clip_empty() const noexcept;

    /** Sets the current font for text operations.
        
        @param font The font to use for subsequent text drawing
    */
    void set_font (const Font& font);

    /** Sets the current font with a specified height.
        
        @param height The height of the font in pixels
    */
    void set_font (double height);

    /** Sets the current fill to a solid color.
        
        @param color The color to use for fill operations
    */
    void set_color (Color color);

    /** Fills the current path with the current fill color.
        
        @param path The path to fill
    */
    void fill_path (const Path& path);

    /** Fills a rectangle with the current color at the specified position and size.
        
        @param x The x-coordinate of the top-left corner of the rectangle
        @param y The y-coordinate of the top-left corner of the rectangle
        @param width The width of the rectangle in pixels
        @param height The height of the rectangle in pixels
    */
    void fill_rect (float x, float y, float width, float height);

    /** Fills a rectangle with the current color using integer coordinates.
        
        @param x The x-coordinate of the top-left corner of the rectangle
        @param y The y-coordinate of the top-left corner of the rectangle
        @param width The width of the rectangle in pixels
        @param height The height of the rectangle in pixels
    */
    void fill_rect (int x, int y, int width, int height);

    /** Fills a rectangle from floating-point bounds.
        
        @param r The rectangular bounds to fill
    */
    void fill_rect (const Rectangle<float>& r);

    /** Fills a rectangle from integer bounds.
        
        @param r The rectangular bounds to fill
    */
    void fill_rect (const Rectangle<int>& r);

    /** Draws a rectangle outline at the specified position and size using
        the current stroke settings. The rectangle is drawn with no fill,
        only the outline is rendered.
        
        @param x The x-coordinate of the top-left corner of the rectangle
        @param y The y-coordinate of the top-left corner of the rectangle
        @param width The width of the rectangle in pixels
        @param height The height of the rectangle in pixels
    */
    void draw_rect (float x, float y, float width, float height);

    /** Draws a rectangle outline using integer coordinates.
        
        @param x The x-coordinate of the top-left corner of the rectangle
        @param y The y-coordinate of the top-left corner of the rectangle
        @param width The width of the rectangle in pixels
        @param height The height of the rectangle in pixels
    */
    void draw_rect (int x, int y, int width, int height);

    /** Draws a rectangle outline from floating-point bounds.
        
        @param r The rectangular bounds to draw
    */
    void draw_rect (const Rectangle<float>& r);

    /** Draws a rectangle outline from integer bounds.
        
        @param r The rectangular bounds to draw
    */
    void draw_rect (const Rectangle<int>& r);

    /** Draws a rounded rectangle outline with the current stroke settings.
        
        @param x The x-coordinate of the top-left corner of the rectangle
        @param y The y-coordinate of the top-left corner of the rectangle
        @param width The width of the rectangle in pixels
        @param height The height of the rectangle in pixels
        @param corner_size The radius of the rounded corners in pixels
    */
    void draw_rounded_rect (float x, float y, float width, float height, float corner_size);

    /** Draws a rounded rectangle outline using integer coordinates.
        
        @param x The x-coordinate of the top-left corner of the rectangle
        @param y The y-coordinate of the top-left corner of the rectangle
        @param width The width of the rectangle in pixels
        @param height The height of the rectangle in pixels
        @param corner_size The radius of the rounded corners in pixels
    */
    void draw_rounded_rect (int x, int y, int width, int height, float corner_size);

    /** Draws a rounded rectangle outline from floating-point bounds.
        
        @param r The rectangular bounds to draw
        @param corner_size The radius of the rounded corners in pixels
    */
    void draw_rounded_rect (const Rectangle<float>&, float corner_size);

    /** Draws a rounded rectangle outline from integer bounds.
        
        @param r The rectangular bounds to draw
        @param corner_size The radius of the rounded corners in pixels
    */
    void draw_rounded_rect (const Rectangle<int>& r, float corner_size);

    /** Fills a rounded rectangle with the current color.
        
        @param x The x-coordinate of the top-left corner of the rectangle
        @param y The y-coordinate of the top-left corner of the rectangle
        @param width The width of the rectangle in pixels
        @param height The height of the rectangle in pixels
        @param corner_size The radius of the rounded corners in pixels
    */
    void fill_rounded_rect (float x, float y, float width, float height, float corner_size);

    /** Fills a rounded rectangle using integer coordinates.
        
        @param x The x-coordinate of the top-left corner of the rectangle
        @param y The y-coordinate of the top-left corner of the rectangle
        @param width The width of the rectangle in pixels
        @param height The height of the rectangle in pixels
        @param corner_size The radius of the rounded corners in pixels
    */
    void fill_rounded_rect (int x, int y, int width, int height, float corner_size);

    /** Fills a rounded rectangle from floating-point bounds.
        
        @param r The rectangular bounds to fill
        @param corner_size The radius of the rounded corners in pixels
    */
    void fill_rounded_rect (const Rectangle<float>&, float corner_size);

    /** Fills a rounded rectangle from integer bounds.
        
        @param r The rectangular bounds to fill
        @param corner_size The radius of the rounded corners in pixels
    */
    void fill_rounded_rect (const Rectangle<int>& r, float corner_size);

    /** Strokes the current path with the current stroke settings.
        
        @param path The path to stroke
    */
    void stroke_path (const Path& path);

    /** Draws text within a rectangle with the specified alignment.
        
        @param text The text string to draw
        @param area The rectangular area for the text
        @param align The justification mode for positioning the text
    */
    void draw_text (const std::string& text, Rectangle<float> area, Justify align);

    /** Draws an image within a target rectangle with the specified fitment.
        
        @param image The image to draw
        @param target The target rectangle where the image will be drawn
        @param align The fitment mode for positioning and scaling the image
    */
    void draw_image (Image image, Rectangle<double> target, Fitment align);

    /** Draws an image using a transformation matrix.
        
        @param image The image to draw
        @param transform The transformation to apply to the image
    */
    void draw_image (Image image, Transform transform);

private:
    DrawingContext& _context;
    LUI_DISABLE_COPY (Graphics)
};

} // namespace lui
