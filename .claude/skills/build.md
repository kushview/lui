<!--
SPDX-FileCopyrightText: 2026 Michael Fisher <mfisher@lvtk.org>
SPDX-License-Identifier: ISC
-->

# Build Skills

## Building LUI

**Primary build system**: CMake + Ninja

```bash
# Configure (from project root)
cmake -B build -G Ninja

# Build everything
cd build && ninja -j4

# Build specific targets
ninja lui-demo        # Demo app only
ninja lui-unit-tests  # Unit tests only
```

## Running

```bash
# Run demo
./build/demo/lui-demo

# Run on macOS as app bundle
./build/LUI.app/Contents/MacOS/lui-demo

# Run tests
./build/test/lui-unit-tests
```

## Common Issues

- **Missing dependencies**: Check CMake output for cairo, pugl, boost
- **Build errors in demo/**: Usually need to run `cmake ..` again after file renames
- **Linker errors**: Check that all source files are listed in CMakeLists.txt

## Project Structure

```
include/lui/          - Public headers
src/ui/              - Core implementation + backends
  nanovg/            - NanoVG library
  nanovg.cpp/.hpp    - NanoVG backend implementation
  graphics.cpp       - High-level graphics API
demo/                - Demo application
test/                - Unit tests
```
