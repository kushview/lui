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
    observer_test.cpp
    string_test.cpp
    weak_ref_test.cpp
    ui_font_test.cpp
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
    GTest::gtest
    GTest::gtest_main
)

# Register tests with gtest
gtest_discover_tests(lui-unit)

