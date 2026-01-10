cmake_minimum_required(VERSION 3.20)

project(lui
    VERSION 0.0.1
    LANGUAGES CXX C
    DESCRIPTION "LUI - A Modern C++ UI Library"
)

# Require C++20 and C99
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add Objective-C/C++ support on macOS
if(APPLE)
    enable_language(OBJCXX)
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

# Try to find Boost
# First try standard CMake modules, then pkg-config
find_package(Boost QUIET)
if(NOT Boost_FOUND)
    pkg_check_modules(BOOST boost)
    # If still not found, try homebrew path on macOS
    if(NOT BOOST_FOUND AND APPLE)
        list(APPEND CMAKE_PREFIX_PATH /opt/homebrew/opt/boost)
        find_package(Boost QUIET)
        if(Boost_FOUND)
            set(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
        endif()
    endif()
endif()

# Find Cairo (optional for now)
pkg_check_modules(CAIRO cairo>=1.16.0)

# Find Pugl (optional for now)
pkg_check_modules(PUGL pugl-0>=0.4.0)
pkg_check_modules(PUGL_CAIRO pugl-cairo-0>=0.4.0)

# Python 3 for scripts
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Add subdirectories
add_subdirectory(src)

# Demo
option(LUI_BUILD_DEMO "Build the demo application" ON)
if(LUI_BUILD_DEMO)
    add_subdirectory(demo)
endif()

# Tests
option(LUI_BUILD_TESTS "Build unit tests" ON)
if(LUI_BUILD_TESTS)
    # enable_testing()
    # add_subdirectory(test)
endif()

# Documentation
option(LUI_BUILD_DOCS "Build documentation" ON)
if(LUI_BUILD_DOCS)
    # add_subdirectory(doc)
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
message(STATUS "  Boost: ${BOOST_FOUND}")
message(STATUS "  Python3: ${Python3_EXECUTABLE}")
message(STATUS "")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
