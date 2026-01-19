# Clipping Exclusion Issue

## Problem
When `LUI_DISABLE_CLIPPING` is set to `0` (clipping enabled), hovering over buttons in the demo paints white regions instead of the correct content. When `LUI_DISABLE_CLIPPING` is set to `1`, rendering works correctly.

## Root Cause
The rendering system's clip exclusion mechanism is not fully implemented across backends.

### Affected Files

#### [src/cairo.cpp](../../src/cairo.cpp#L148-L149)
Lines 148-149 disable the entire `exclude_clip` implementation:
```cpp
void exclude_clip (const Rectangle<int>& r) override {
#if 1 // FIXME: exlusions still aren't quite right.
    lui::ignore (r);
#else
    // ... complex even-odd fill rule implementation ...
#endif
}
```

The comment "FIXME: exlusions still aren't quite right" indicates the implementation was attempted but incomplete.

#### [src/core_graphics.cpp](../../src/core_graphics.cpp#L145-L147)
Lines 145-147 stub out the implementation entirely:
```cpp
void exclude_clip (const Rectangle<int>& r) override {
    // TODO: Implement clip exclusion for CoreGraphics
    lui::ignore (r);
}
```

### How It's Used

[src/widget.cpp](../../src/widget.cpp#L125-L169) in the `render_all` method:
- Clips the child widget's bounds (line 154): `g.clip (tb);`
- Then excludes (punches holes for) all opaque widgets above it in the layer order (lines 159-165)
- Calls `exclude_clip` to prevent those opaque widgets from being painted underneath

When `exclude_clip` is stubbed out, the clip region is never properly modified to exclude the opaque widgets, causing the wrong content to be painted in those areas.

## Current Behavior
- **With clipping disabled** (`LUI_DISABLE_CLIPPING = 1`): Everything works because the `repaint` method skips clip regions entirely and calls `puglPostRedisplay` without bounds, repainting the whole view
- **With clipping enabled** (`LUI_DISABLE_CLIPPING = 0`): Clip exclusion fails silently, leading to visual artifacts (white/transparent areas)

## Solution Path
Implement proper `exclude_clip` for both:
1. Cairo: Properly handle the even-odd fill rule to create holes in the clip region
2. CoreGraphics: Use appropriate macOS graphics APIs to exclude regions from the current clip

The previous Cairo attempt used `cairo_set_fill_rule` with `CAIRO_FILL_RULE_EVEN_ODD` but there may be issues with:
- How the clip state is tracked (only stores a single rectangle, not complex shapes)
- Proper restoration of fill rule after clipping
- Interaction with the graphics state stack
