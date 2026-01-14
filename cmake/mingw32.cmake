# SPDX-FileCopyrightText: 2026 Kushview, LLC
# SPDX-License-Identifier: ISC

# MinGW-w64 cross-compilation toolchain file for macOS -> Windows

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Where to find target environment
# Let CMake automatically detect the target environment from the compiler
# set(CMAKE_FIND_ROOT_PATH /opt/homebrew/opt/mingw-w64)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Make sure we link statically to avoid runtime DLL dependencies
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-static")
