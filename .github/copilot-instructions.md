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
