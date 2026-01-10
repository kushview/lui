# LUI Architecture

## Design Philosophy

LUI follows a **JUCE-inspired** architecture with backend abstraction:

- Core library is backend-agnostic
- Path operations designed for safety (defensive programming)
- Widgets use high-level Graphics API
- Backend handles renderer-specific quirks

## Layer Responsibilities

### 1. Core Abstractions (`include/lui/`)

**Path** - Geometric path representation
- Stores operations as float array (type, coords)
- Adds defensive move_to(0,0) if geometry ops called on empty path
- JUCE-compatible behavior by design
- **Do not modify** - this is intentional safety feature

**Graphics** - High-level drawing API
- `fill_path()`, `stroke_path()`, `fill_rect()`, etc.
- Iterates Path operations and delegates to DrawingContext
- Backend-agnostic

**DrawingContext** - Abstract interface
- Pure virtual methods for drawing operations
- Implemented by backend (NanoVG, Cairo, etc.)

### 2. Backend Implementation (`src/ui/nanovg.cpp`)

**Context** - NanoVG DrawingContext implementation
- Maps abstract operations to NanoVG calls
- Handles NanoVG-specific quirks
- Tracks state (`has_geometry`, `last_pos`)
- **This is where fixes belong** for rendering issues

**Key methods:**
- `clear_path()` - nvgBeginPath + nvgPathWinding
- `move_to()` - Filters spurious 0,0 moves
- `fill()`, `stroke()` - Call nvgFill/nvgStroke

### 3. Widgets (`src/ui/*.cpp`)

- Slider, Dial, Button, etc.
- Override `paint(Graphics& g)` method
- Use Graphics high-level API only
- Should not know about NanoVG

## Data Flow Example

```
Dial::paint()
  → Graphics::fill_path(Path)
    → Context::clear_path()  [nvgBeginPath]
    → Context::move_to()     [nvgMoveTo, filtered]
    → Context::cubic_to()    [nvgBezierTo]
    → Context::close_path()  [nvgClosePath]
    → Context::fill()        [nvgFill]
```

## Debugging Strategy

1. **Rendering bug?** → Check backend implementation first
2. **Path looks wrong?** → Verify Path data with debug output
3. **Widget behavior off?** → Check widget paint() logic
4. **Only modify backend** unless core design is fundamentally flawed

## Platform Support

- ✅ macOS (tested)
- ⚠️  Linux (untested)
- ⚠️  Windows (untested)

## Dependencies

- NanoVG (bundled in `src/nanovg/`)
- OpenGL (GL2 or GL3 via compile flags)
- CMake 3.15+
- C++20 compiler
