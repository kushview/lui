<!--
SPDX-FileCopyrightText: 2026 Kushview, LLC
SPDX-License-Identifier: ISC
-->

# Copilot Instructions for LUI Project

## Copyright Notice

- All new files should use: `Copyright 2026 Kushview, LLC`
- Do not use individual contributor names (e.g., Michael Fisher, lvtk.org) in copyright headers.

## General Guidelines

- This is a GUI toolkit library. Changes to public APIs affect all users.
- **Do not suggest changing working code without evidence of an actual problem** (bugs, profiling data showing performance issues, etc.)
- When asked about optimization, first assess whether there's a real performance problem before suggesting changes
- Verify current behavior is correct before proposing alternatives
- Check how code is used throughout the codebase before suggesting semantic changes

## Code Review Approach

When reviewing code:
1. First state if the code looks correct and clean
2. Only suggest changes if there's clear evidence of:
   - A bug or incorrect behavior
   - Actual performance bottleneck (with profiling data)
   - Missing functionality explicitly requested
3. Avoid "theoretical improvements" or "different ways to do things" without concrete justification

## Testing

- Write comprehensive unit tests for new functionality
- Ensure tests validate expected behavior without assuming implementation is wrong
- All tests should pass before considering any refactoring

## Performance

- Don't hunt for optimizations without profiling data
- The compiler already optimizes simple inline operations well
- Only optimize when there's measured evidence of a bottleneck

## Code Documentation
### C++ Doxygen
- Use Doxygen comments
- Ensure a space is between description text and first `@param` or other tag.
**Example**
```c++
/** Checks if a rectangular region is obscured or hidden.
    This method tests whether the specified area intersects with any
    obscuring elements in the current view hierarchy.
        
    @param x The x-coordinate of the region's top-left corner
    @param y The y-coordinate of the region's top-left corner
    @param width The width of the region to test
    @param height The height of the region to test
    @return true if any part of the region is obscured, false otherwise
*/
bool obscructed (int x, int y, int width, int height);
```

## Graphics Backend Implementation Notes

### Direct2D Backend (src/direct2d.cpp, src/win_direct2d.cpp)

**Critical Issues & Solutions:**

1. **Matrix Multiplication Order**
   - Direct2D matrix multiplication `A * B` applies B's transform FIRST, then A's transform
   - When composing transforms: use `newTransform * currentTransform` to apply current first, then new
   - This affects both `transform()` and `draw_image()` methods
   - Wrong order causes rotation/translation to apply backwards (e.g., dial indicators gliding instead of rotating)

2. **Text Positioning**
   - `move_to()` MUST always update `current_pos` even when `geometrySink` is null
   - Graphics layer calls `move_to()` to set baseline position before `show_text()`
   - If `current_pos` isn't updated, text renders at (0,0) instead of intended position
   - DirectWrite uses top-left positioning, so subtract ascent from baseline: `y - ascent`

3. **Font Metrics COM Lifetime**
   - Must check if metrics were retrieved BEFORE releasing COM objects
   - Use a boolean flag (e.g., `gotMetrics`) set before `SafeRelease()` calls
   - Checking `if (!font)` after `SafeRelease(&font)` always returns true (null pointer)

4. **Transform Save/Restore**
   - Must store `D2D1_MATRIX_3X2_F` in State struct for proper save/restore
   - Call `rt->GetTransform()` in `save()` and `rt->SetTransform()` in `restore()`
   - Without this, transforms compound incorrectly across save/restore boundaries

5. **Clipping Management**
   - Track clip depth with counter (`clip_depth`) in State struct
   - `PushAxisAlignedClip()` increments depth, `PopAxisAlignedClip()` decrements
   - In `restore()`, pop any extra clips pushed since the corresponding `save()`
   - Initial frame clip counts as depth 1, cleaned up in `end_frame()`

6. **Path Geometry Lifecycle**
   - `ID2D1PathGeometry` is device-independent, created from factory
   - `ID2D1GeometrySink` is obtained from `PathGeometry->Open()`
   - Must call `BeginFigure()` before adding points, `EndFigure()` before drawing
   - Track `figure_active` boolean to ensure proper figure state
   - Call `geometrySink->Close()` before rendering geometry
   - Geometry coordinates are NOT transformed; transform is applied during `DrawGeometry()`

7. **DPI Handling**
   - Use 96 DPI (identity mapping) in render target creation for 1:1 pixel coordinates
   - This matches pugl's window size and mouse coordinate reporting
   - Set both `dpiX` and `dpiY` to 96.0f in `D2D1_RENDER_TARGET_PROPERTIES`

8. **Resource Management**
   - Device-independent: `ID2D1Factory`, `IDWriteFactory` (created once, persist)
   - Device-dependent: `ID2D1HwndRenderTarget` (recreated on window resize)
   - Handle `D2DERR_RECREATE_TARGET` from `EndDraw()` by discarding device resources
   - Use `SafeRelease()` template helper for consistent COM cleanup

### GDI Backend (src/gdi.cpp, src/win_gdi.c)

**Implementation Notes:**

1. **GDI+ Transform Handling**
   - `Graphics::MultiplyTransform()` multiplies current * new (same semantics as Direct2D)
   - `GraphicsPath` accumulates points in local coordinates, transformed when drawn
   - `Graphics::Save()` and `Graphics::Restore()` manage full graphics state including transform

2. **Font Loading from Memory**
   - Use `Gdiplus::PrivateFontCollection` for loading fonts from memory
   - Call `AddMemoryFont()` with embedded font data (e.g., Roboto-Regular.h)
   - Fonts persist for the lifetime of the PrivateFontCollection object
   - Example pattern to follow when implementing custom font loading in Direct2D

3. **Resource Cleanup**
   - GDI+ uses smart pointers (`std::unique_ptr`) for automatic cleanup
   - `Graphics`, `GraphicsPath`, `PrivateFontCollection` auto-release
   - Manual cleanup still needed for some GDI handles (HDC, HBITMAP)

4. **Coordinate System**
   - GDI+ uses top-left origin like Direct2D (no flipping needed)
   - Natural mapping to LUI's coordinate system
   - Text baseline handling similar to Direct2D approach

**Backend Comparison:**
- Direct2D: Lower-level, more control, explicit resource management, hardware-accelerated
- GDI+: Higher-level, more automatic, easier font handling, CPU-rendered
- Both use row-major 3x2 affine transformation matrices
- Both require careful attention to matrix multiplication order
