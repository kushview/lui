// Copyright 2024 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

/**
    Coordinate System Strategy:

    The Pugl backend (mac_cg.m) applies a global Y-flip transformation to the
    CoreGraphics context, converting from CG's native bottom-left origin to
    match NSView's top-left flipped coordinate system. This means all standard
    drawing operations (paths, fills, strokes, rectangles) work naturally with
    top-left coordinates.

    However, Core Text and CGImage have internal coordinate systems that expect
    bottom-left origins. To render them correctly, we locally "un-flip" the
    coordinate system using CGContextSaveGState/RestoreGState for just those
    operations (show_text and draw_image). This is cleaner than flipping
    coordinates for every drawing operation throughout the codebase.
*/

#include <cassert>
#include <iostream>

#include <ApplicationServices/ApplicationServices.h>

#include "mac_cg.h"

#define LUI_CG_DEFAULT_FONT "Helvetica"

#include <lui/core_graphics.hpp>
#include <lui/graphics.hpp>
#include <lui/widget.hpp>

namespace lui {
namespace cg {

class Context : public DrawingContext {
public:
    explicit Context (CGContextRef context = nullptr)
        : cg (context) {
        stack.reserve (64);
    }

    ~Context() {
        cg = nullptr;
    }

    bool begin_frame (CGContextRef _cg, lui::Bounds bounds) {
        cg    = _cg;
        state = {};
        stack.clear();
        this->clip (bounds);
        return true;
    }

    void end_frame() {
        cg = nullptr;
    }

    double device_scale() const noexcept override {
        assert (cg != nullptr);
        // CoreGraphics uses user space scaling
        CGAffineTransform ctm = CGContextGetUserSpaceToDeviceSpaceTransform (cg);
        return static_cast<double> (ctm.a); // x scale factor
    }

    void save() override {
        CGContextSaveGState (cg);
        stack.push_back (state);
    }

    void restore() override {
        CGContextRestoreGState (cg);
        if (stack.empty())
            return;
        std::swap (state, stack.back());
        stack.pop_back();
    }

    void set_line_width (double width) override {
        CGContextSetLineWidth (cg, width);
    }

    void clear_path() override {
        CGContextBeginPath (cg);
    }

    void move_to (double x1, double y1) override {
        CGContextMoveToPoint (cg, x1, y1);
    }

    void line_to (double x1, double y1) override {
        CGContextAddLineToPoint (cg, x1, y1);
    }

    void quad_to (double x1, double y1, double x2, double y2) override {
        CGContextAddQuadCurveToPoint (cg, x1, y1, x2, y2);
    }

    void cubic_to (double x1, double y1, double x2, double y2, double x3, double y3) override {
        CGContextAddCurveToPoint (cg, x1, y1, x2, y2, x3, y3);
    }

    void close_path() override {
        CGContextClosePath (cg);
    }

    /** Fill the current path with the currrent settings */
    void fill() override {
        apply_pending_state();
        CGContextFillPath (cg);
    }

    /** Stroke the current path with current settings */
    void stroke() override {
        apply_pending_state();
        CGContextStrokePath (cg);
    }

    /** Translate the origin */
    void translate (double x, double y) override {
        CGContextTranslateCTM (cg, x, y);
        state.clip.x -= x;
        state.clip.y -= y;
    }

    /** Apply transformation matrix */
    void transform (const Transform& mat) override {
        CGAffineTransform m = CGAffineTransformMake (
            mat.m00, mat.m10, mat.m01, mat.m11, mat.m02, mat.m12);
        CGContextConcatCTM (cg, m);
    }

    void reset_clip() noexcept {
        state.clip = {};
        CGContextResetClip (cg);
    }

    void clip (const Rectangle<int>& r) override {
        state.clip = r.as<double>();
        CGContextBeginPath (cg);
        CGContextAddRect (cg, CGRectMake (r.x, r.y, r.width, r.height));
        CGContextClip (cg);
    }

    void exclude_clip (const Rectangle<int>& r) override {
        // TODO: Implement clip exclusion for CoreGraphics
        lui::ignore (r);
    }

    Rectangle<int> last_clip() const override {
        return state.clip.as<int>();
    }

    Font font() const noexcept override { return state.font; }

    void set_font (const Font& f) override {
        state.font = f;

        // Create CTFont for the given font
        CFStringRef fontName = CFStringCreateWithCString (
            kCFAllocatorDefault, LUI_CG_DEFAULT_FONT, kCFStringEncodingUTF8);

        CTFontRef ctFont = CTFontCreateWithName (fontName, f.height(), nullptr);
        CFRelease (fontName);

        if (ctFont) {
            CGContextSetFont (cg, CGFontCreateWithFontName (CTFontCopyFullName (ctFont)));
            CGContextSetFontSize (cg, f.height());
            CFRelease (ctFont);
        }
    }

    void set_fill (const Fill& fill) override {
        auto c = state.color = fill.color();
        _fill_dirty          = false;
        CGContextSetRGBFillColor (cg, c.fred(), c.fgreen(), c.fblue(), c.alpha());
        CGContextSetRGBStrokeColor (cg, c.fred(), c.fgreen(), c.fblue(), c.alpha());
    }

    void fill_rect (const Rectangle<double>& r) override {
        apply_pending_state();
        CGContextFillRect (cg, CGRectMake (r.x, r.y, r.width, r.height));
    }

    FontMetrics font_metrics() const noexcept override {
        // Create CTFont from current font
        CFStringRef fontName = CFStringCreateWithCString (
            kCFAllocatorDefault, LUI_CG_DEFAULT_FONT, kCFStringEncodingUTF8);

        CTFontRef ctFont = CTFontCreateWithName (fontName, state.font.height(), nullptr);
        CFRelease (fontName);

        FontMetrics fm;
        if (ctFont) {
            fm.ascent  = CTFontGetAscent (ctFont);
            fm.descent = CTFontGetDescent (ctFont);
            fm.height  = fm.ascent + fm.descent;
#if 0
            fm.leading = CTFontGetLeading (ctFont);
#endif       
            // Use bounding box for max stride approximation
            CGRect bbox = CTFontGetBoundingBox (ctFont);
            fm.x_stride_max = bbox.size.width;
            fm.y_stride_max = bbox.size.height;
            CFRelease (ctFont);
        }

        return fm;
    }

    TextMetrics text_metrics (std::string_view text) const noexcept override {
        TextMetrics tm;

        CFStringRef str = CFStringCreateWithBytes (
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*> (text.data()),
            text.size(),
            kCFStringEncodingUTF8,
            false);

        if (! str)
            return tm;

        CFStringRef fontName = CFStringCreateWithCString (
            kCFAllocatorDefault, LUI_CG_DEFAULT_FONT, kCFStringEncodingUTF8);

        CTFontRef ctFont = CTFontCreateWithName (fontName, state.font.height(), nullptr);
        CFRelease (fontName);

        if (ctFont) {
            CFMutableAttributedStringRef attrStr = CFAttributedStringCreateMutable (
                kCFAllocatorDefault, 0);
            CFAttributedStringReplaceString (attrStr, CFRangeMake (0, 0), str);
            CFAttributedStringSetAttribute (
                attrStr, CFRangeMake (0, CFStringGetLength (str)), kCTFontAttributeName, ctFont);

            CTLineRef line = CTLineCreateWithAttributedString (attrStr);
            CGRect bounds  = CTLineGetBoundsWithOptions (line, 0);

            tm.width    = bounds.size.width;
            tm.height   = bounds.size.height;
            tm.x_offset = bounds.origin.x;
            tm.y_offset = bounds.origin.y;
            tm.x_stride = CTLineGetTypographicBounds (line, nullptr, nullptr, nullptr);
            tm.y_stride = 0.0; // Horizontal text has no vertical advance

            CFRelease (line);
            CFRelease (attrStr);
            CFRelease (ctFont);
        }

        CFRelease (str);
        return tm;
    }

    bool show_text (const std::string_view text) override {
        apply_pending_state();

        CFStringRef str = CFStringCreateWithBytes (
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*> (text.data()),
            text.size(),
            kCFStringEncodingUTF8,
            false);

        if (! str)
            return false;

        CFStringRef fontName = CFStringCreateWithCString (
            kCFAllocatorDefault, LUI_CG_DEFAULT_FONT, kCFStringEncodingUTF8);

        CTFontRef ctFont = CTFontCreateWithName (fontName, state.font.height(), nullptr);
        CFRelease (fontName);

        if (ctFont) {
            CFMutableAttributedStringRef attrStr = CFAttributedStringCreateMutable (
                kCFAllocatorDefault, 0);
            CFAttributedStringReplaceString (attrStr, CFRangeMake (0, 0), str);
            CFAttributedStringSetAttribute (
                attrStr, CFRangeMake (0, CFStringGetLength (str)), kCTFontAttributeName, ctFont);

            // Set text color
            CGColorRef color = CGColorCreateGenericRGB (
                state.color.fred(), state.color.fgreen(), state.color.fblue(), state.color.alpha());
            CFAttributedStringSetAttribute (
                attrStr, CFRangeMake (0, CFStringGetLength (str)), kCTForegroundColorAttributeName, color);
            CGColorRelease (color);

            CTLineRef line = CTLineCreateWithAttributedString (attrStr);

            // Get current point from path (where move_to positioned us)
            CGPoint currentPoint = CGContextGetPathCurrentPoint (cg);

            CGContextSaveGState (cg);
            // Translate to position, flip Y for text
            CGContextTranslateCTM (cg, currentPoint.x, currentPoint.y);
            CGContextScaleCTM (cg, 1.0, -1.0);

            // Explicitly set text position to origin after transforms
            CGContextSetTextPosition (cg, 0, 0);

            CTLineDraw (line, cg);

            CGContextRestoreGState (cg);

            CFRelease (line);
            CFRelease (attrStr);
            CFRelease (ctFont);
        }

        CFRelease (str);
        return true;
    }

    void draw_image (Image i, Transform matrix) override {
        CGImageAlphaInfo alphaInfo;
        CGBitmapInfo bitmapInfo;
        size_t bitsPerComponent = 8;
        size_t bitsPerPixel;

        switch (i.format()) {
            case PixelFormat::ARGB32:
                alphaInfo    = kCGImageAlphaFirst;
                bitmapInfo   = kCGBitmapByteOrder32Host | alphaInfo;
                bitsPerPixel = 32;
                break;
            case PixelFormat::RGB24:
                alphaInfo    = kCGImageAlphaNoneSkipFirst;
                bitmapInfo   = kCGBitmapByteOrder32Host | alphaInfo;
                bitsPerPixel = 32;
                break;
            case PixelFormat::INVALID:
            default:
                return;
        }

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CGDataProviderRef provider = CGDataProviderCreateWithData (
            nullptr, i.data(), i.stride() * i.height(), nullptr);

        CGImageRef image = CGImageCreate (
            i.width(), i.height(), bitsPerComponent, bitsPerPixel, i.stride(), colorSpace, bitmapInfo, provider, nullptr, false, kCGRenderingIntentDefault);

        if (image) {
            CGContextSaveGState (cg);

            transform (matrix);

            // Images need to be flipped since we've already flipped the context
            CGContextTranslateCTM (cg, 0, i.height());
            CGContextScaleCTM (cg, 1.0, -1.0);

            CGContextDrawImage (cg, CGRectMake (0, 0, i.width(), i.height()), image);

            CGContextRestoreGState (cg);
            CGImageRelease (image);
        }

        CGDataProviderRelease (provider);
        CGColorSpaceRelease (colorSpace);
    }

private:
    void apply_pending_state() {
        if (_fill_dirty) {
            set_fill (Fill { state.color });
        }
    }

    struct State {
        Font font;
        Color color;
        Rectangle<double> clip;
    };

    CGContextRef cg { nullptr };
    State state;
    std::vector<State> stack;
    bool _fill_dirty { false };
};

class View : public lui::View {
public:
    View (Main& m, Widget& w)
        : lui::View (m, w) {
        set_backend ((uintptr_t) puglCGBackend());
        set_view_hint (PUGL_DOUBLE_BUFFER, PUGL_FALSE);
        set_view_hint (PUGL_RESIZABLE, PUGL_TRUE);
        puglSetViewString ((PuglView*) c_obj(), PUGL_WINDOW_TITLE, w.name().c_str());
    }

    ~View() {}

    void expose (Bounds frame) override {
        auto cg = (CGContextRef) puglGetContext (_view);
        assert (cg != nullptr);

        if (true || ! _scale_set || _last_scale != scale_factor()) {
            _scale_set         = true;
            const auto scale_x = scale_factor();
            const auto scale_y = scale_x;
            _last_scale        = scale_x;
        }

        CGContextSaveGState (cg);

        if (_context->begin_frame (cg, frame)) {
            render (*_context);
            _context->end_frame();
        }

        CGContextRestoreGState (cg);
    }

    void created() override {
        _context = std::make_unique<Context>();
        _view    = (PuglView*) c_obj();
        assert (_view != nullptr && _context != nullptr);
    }

    void destroyed() override {
        _view = nullptr;
        _context.reset();
    }

private:
    using Parent = lui::View;
    PuglView* _view;
    std::unique_ptr<Context> _context;
    bool _scale_set { false };
    double _last_scale { 1.0 };
};

} // namespace cg

std::unique_ptr<lui::View> CoreGraphics::create_view (Main& c, Widget& w) {
    return std::make_unique<cg::View> (c, w);
}

} // namespace lui
