#!/bin/bash
# SPDX-FileCopyrightText: 2026 Kushview, LLC
# SPDX-License-Identifier: ISC
# Format C++ source files using clang-format version 21

set -e

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format not found"
    exit 1
fi

# Get clang-format version
VERSION=$(clang-format --version | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | cut -d. -f1)

if [ "$VERSION" != "21" ]; then
    echo "Error: clang-format version 21 required, found version $VERSION"
    echo "Install with: brew install llvm@21"
    echo "Then ensure clang-format points to version 21"
    exit 2
fi

echo "Using clang-format version $VERSION"

# Find and format all C++ source and header files
find src include demo -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.c" -o -name "*.m" -o -name "*.mm" \) \
    -not -path "*/pugl/*" \
    -not -path "*/nanovg/*" \
    -not -path "*/stb/*" \
    -not -path "*/sol/*" \
    -exec clang-format -i {} +

echo "Formatting complete"
