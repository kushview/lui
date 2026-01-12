cmake_minimum_required(VERSION 3.20)

project(lui
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "LUI - A Modern C++ UI Library"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Require C++20 and C99
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add Objective-C/C++ support on macOS
if(APPLE)
    enable_language(OBJCXX)
endif()

# Option for Node.js bindings compatibility
option(LUI_NODE "Build for Node.js bindings compatibility" OFF)

# Set macOS deployment target for Node.js compatibility
if(LUI_NODE AND APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3" CACHE STRING "Minimum macOS deployment version for Node.js" FORCE)
    message(STATUS "Building for Node.js: macOS deployment target set to ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Version info
set(LUI_ABI_VERSION "0.0")
set(LUI_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(LUI_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(LUI_VERSION_MICRO ${PROJECT_VERSION_PATCH})

# Find required packages
find_package(Threads REQUIRED)

# Use pkgconfig to find dependencies
include(FindPkgConfig)
include(GNUInstallDirs)

# Find Cairo (optional for now)
option(LUI_BUILD_CAIRO "Build the Cairo backend if available" ON)
if(LUI_BUILD_CAIRO)
    set(ENV{PKG_CONFIG_PATH} "/opt/lui/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    pkg_check_modules(CAIRO cairo>=1.16.0)
else()
    set(CAIRO_FOUND FALSE)
endif()

# Python 3 for scripts
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Add subdirectories
add_subdirectory(src)

find_program(LUA_EXE lua)
if(LUA_EXE)
    # Run `lua -v` extract version number
    execute_process(
        COMMAND ${LUA_EXE} -v
        OUTPUT_VARIABLE LUA_VERSION_OUTPUT
        ERROR_VARIABLE LUA_VERSION_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX MATCH "Lua ([0-9]+\\.[0-9]+)" LUA_VERSION_MATCH "${LUA_VERSION_OUTPUT}")
    if(LUA_VERSION_MATCH)
        set(LUA_VERSION ${CMAKE_MATCH_1})
        message(STATUS "Lua interpreter found: ${LUA_EXE} (version ${LUA_VERSION})")
    else()
        message(STATUS "Lua interpreter found, but does not report version: ${LUA_EXE}")
        unset(LUA_EXE)
        unset(LUA_VERSION)
    endif()
endif()

if(CAIRO_FOUND AND LUA_EXE)
    add_subdirectory(bindings/lua)
endif()

# Demo
option(LUI_BUILD_DEMO "Build the demo application" ON)
if(LUI_BUILD_DEMO)
    add_subdirectory(demo)
endif()

# Tests
option(LUI_BUILD_TESTS "Build unit tests" ON)
if(LUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# Documentation
option(LUI_BUILD_DOCS "Build documentation" ON)
if(LUI_BUILD_DOCS)
    add_subdirectory(doc)
endif()

# Install headers
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/lui"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/lui-${LUI_ABI_VERSION}"
)

# Print summary
message(STATUS "")
message(STATUS "LUI Build Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Demo: ${LUI_BUILD_DEMO}")
message(STATUS "  Build Tests: ${LUI_BUILD_TESTS}")
message(STATUS "  Build Docs: ${LUI_BUILD_DOCS}")
message(STATUS "  Cairo: ${CAIRO_FOUND}")
message(STATUS "  Pugl: ${PUGL_FOUND}")
message(STATUS "  Python3: ${Python3_EXECUTABLE}")
message(STATUS "")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
