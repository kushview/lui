<!--
SPDX-FileCopyrightText: 2026 Kushview, LLC
SPDX-License-Identifier: ISC
-->

# Copilot Instructions for LUI Project

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
