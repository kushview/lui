# Unit tests
if(Boost_FOUND)
    set(UNIT_TEST_SOURCES
        observer_test.cpp
        string_test.cpp
        weak_ref_test.cpp
        ui_font_test.cpp
        main_unit.cpp
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
        ${Boost_LIBRARIES}
        lui-${LUI_ABI_VERSION}
    )

    # Register tests
    add_test(NAME Font
        COMMAND lui-unit -t Font
        ENVIRONMENT "LV2_PATH=${CMAKE_BINARY_DIR}"
    )
    add_test(NAME Observer
        COMMAND lui-unit -t Observer
        ENVIRONMENT "LV2_PATH=${CMAKE_BINARY_DIR}"
    )
    add_test(NAME String
        COMMAND lui-unit -t String
        ENVIRONMENT "LV2_PATH=${CMAKE_BINARY_DIR}"
    )
    add_test(NAME WeakRef
        COMMAND lui-unit -t WeakRef
        ENVIRONMENT "LV2_PATH=${CMAKE_BINARY_DIR}"
    )
endif()
