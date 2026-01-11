# Demo resources
set(DEMO_RES_FILES
    res/lv2_logo.png
)

set(DEMO_RES_GENERATED)
foreach(resource ${DEMO_RES_FILES})
    get_filename_component(resource_name ${resource} NAME)
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${resource_name}.h")
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/scripts/bin2c.py"
                ${CMAKE_CURRENT_SOURCE_DIR}/${resource} ${output} ${CMAKE_CURRENT_SOURCE_DIR}/${resource}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${resource}
        COMMENT "Generating ${resource_name}.h"
    )
    list(APPEND DEMO_RES_GENERATED ${output})
endforeach()

# Demo library sources
set(LIBLUI_DEMO_SOURCES
    demo.cpp
    dials.cpp
    entry.cpp
    images.cpp
    sliders.cpp
    shapes.cpp
    ${DEMO_RES_GENERATED}
)

# Create demo library (static)
add_library(lui-demo STATIC ${LIBLUI_DEMO_SOURCES})

target_include_directories(lui-demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_directories(lui-demo PRIVATE
    ${CAIRO_LIBRARY_DIRS}
)

target_link_libraries(lui-demo PRIVATE
    lui-${LUI_ABI_VERSION}
    Threads::Threads
)

# Demo executable
add_executable(lui-demo-exe
    main.cpp
)

set_target_properties(lui-demo-exe PROPERTIES
    OUTPUT_NAME lui-demo
)

if(APPLE)
    set_target_properties(lui-demo-exe PROPERTIES
        OUTPUT_NAME LUI
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "LUI"
        MACOSX_BUNDLE_GUI_IDENTIFIER "net.kushview.lui.demo"
        MACOSX_BUNDLE_EXECUTABLE_NAME "LUI"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) Kushview, LLC"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
endif()

target_include_directories(lui-demo-exe PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(lui-demo-exe PRIVATE
    LUI_NO_SYMBOL_EXPORT
    LUI_DEMO_OPENGL=1
)

target_link_libraries(lui-demo-exe PRIVATE
    lui-demo
)

# Install
install(TARGETS lui-demo-exe
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION Applications
)
