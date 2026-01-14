<!--
SPDX-FileCopyrightText: 2026 Kushview, LLC
SPDX-License-Identifier: ISC
-->

# NanoVG Backend Knowledge

## Critical Implementation Details

## NanoVG/OpenGL Backend
### NanoVG Path Handling

**Every `nvgMoveTo` creates a new sub-path**
- Unlike some graphics APIs, `nvgMoveTo` doesn't just reposition
- Creates a separate path in the NanoVG command buffer
- Multiple sub-paths can interfere with fills

**Path lifecycle:**
1. `nvgBeginPath()` - Clear all paths, start fresh
2. `nvgMoveTo(x, y)` - Start new sub-path at x,y
3. `nvgLineTo/nvgBezierTo/etc` - Add to current sub-path
4. `nvgMoveTo(x2, y2)` - Start ANOTHER sub-path
5. `nvgFill()` - Tesselate and fill ALL sub-paths

### Fill Rule

Despite docs claiming "even-odd", NanoVG uses **non-zero fill rule**:
- Winding direction matters (CW vs CCW)
- `nvgPathWinding(NVG_SOLID)` = CCW (counter-clockwise)
- `nvgPathWinding(NVG_HOLE)` = CW (clockwise)
- Empty sub-paths can cause artifacts

### Winding Enforcement

In `nvg__flattenPaths()`, NanoVG:
1. Tessellates bezier curves to line segments
2. Calculates polygon area to detect winding
3. **Reverses polygon if winding doesn't match `path->winding`**

This means you can draw CW and set `NVG_CCW` - NanoVG will reverse it.

### Bezier Approximation

- NanoVG uses `NVG_KAPPA90 = 0.5522847498` for circular bezier curves
- This is the mathematically correct constant for 4-bezier circle approximation
- Generic approximations using 0.55 are close but not exact

## Our Implementation Strategy

**In `Context::move_to()`:**
```cpp
// Skip spurious move_to(0,0) if we haven't added geometry yet
if (!ctx->has_geometry && x1 == 0.0 && y1 == 0.0) {
    return;  // Don't create empty sub-path
}
```

**Track geometry state:**
- `has_geometry` flag set by `line_to`, `cubic_to`, `quad_to`
- Cleared by `clear_path()`
- Prevents Path's defensive move_to(0,0) from creating empty sub-paths

## References

- GitHub Issue #598: Fill rule confusion (confirms non-zero fill)
- NanoVG source: `nanovg.c` lines 1332-1430 (path flattening)
- `NVG_KAPPA90` definition for optimal circular beziers
