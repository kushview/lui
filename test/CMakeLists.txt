# Unit tests
# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Import gtest_discover_tests
include(GoogleTest)

set(UNIT_TEST_SOURCES
    fitment_test.cpp
    observer_test.cpp
    string_test.cpp
    transform_test.cpp
    weak_ref_test.cpp
    font_test.cpp
)

add_executable(lui-unit ${UNIT_TEST_SOURCES})

set_target_properties(lui-unit PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

target_compile_definitions(lui-unit PRIVATE
    LUI_NO_SYMBOL_EXPORT
)

target_link_libraries(lui-unit PRIVATE
    lui-${LUI_ABI_VERSION}
    GTest::gtest_main
)

# Register tests with gtest
gtest_discover_tests(lui-unit)

# Add REUSE license compliance check
find_program(REUSE_EXE NAMES reuse)
if(REUSE_EXE)
    add_test(
        NAME reuse-lint
        COMMAND ${REUSE_EXE} lint
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    message(STATUS "REUSE found - license compliance test added")
else()
    message(STATUS "REUSE not found - license compliance test skipped")
endif()
