# Demo resources
set(demo_res_files
    res/lv2_logo.png
)

set(demo_res_generated)
foreach(resource ${demo_res_files})
    get_filename_component(resource_name ${resource} NAME)
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${resource_name}.h")
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/scripts/bin2c.py"
                ${CMAKE_CURRENT_SOURCE_DIR}/${resource} ${output} ${CMAKE_CURRENT_SOURCE_DIR}/${resource}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${resource}
        COMMENT "Generating ${resource_name}.h"
    )
    list(APPEND demo_res_generated ${output})
endforeach()

# Demo library sources
set(LIBLUI_DEMO_SOURCES
    demo.cpp
    dials.cpp
    entry.cpp
    images.cpp
    sliders.cpp
    shapes.cpp
    ${demo_res_generated}
)

# Create demo library (static)
add_library(lui-demo STATIC ${LIBLUI_DEMO_SOURCES})

target_include_directories(lui-demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(lui-demo PRIVATE
    lui-${LUI_ABI_VERSION}
    Threads::Threads)

function(lui_add_demo tgt)
    if(ARGC GREATER 1)
        set(bundle_name ${ARGV1})
    else()
        set(bundle_name ${tgt})
    endif()
    
    if(WIN32)
        add_executable(${tgt} WIN32 main.cpp)
    else()
        add_executable(${tgt} main.cpp)
    endif()
    
    target_include_directories(${tgt} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src)
    target_compile_definitions(${tgt} PRIVATE LUI_NO_SYMBOL_EXPORT)
    
    if(WIN32)
        target_compile_definitions(${tgt} PRIVATE LUI_STATIC)
    endif()
    
    target_link_libraries(${tgt} PRIVATE lui-demo)

    if(APPLE)
        set_target_properties(${tgt} PROPERTIES
            OUTPUT_NAME "${bundle_name}"
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME "${bundle_name}"
            MACOSX_BUNDLE_GUI_IDENTIFIER "net.kushview.lui.demo"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_COPYRIGHT "Copyright (c) Kushview, LLC"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        )
    endif()

    install(TARGETS ${tgt}
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION Applications
    )
endfunction()

if(NOT WIN32)
    lui_add_demo(lui-demo-gl)
    target_compile_definitions(lui-demo-gl PRIVATE LUI_DEMO_OPENGL=1)
    target_link_directories(lui-demo-gl PRIVATE ${CAIRO_LIBRARY_DIRS})
    target_link_libraries(lui-demo-gl PRIVATE lui-gl-${LUI_ABI_VERSION})
endif()

# Cairo
if(CAIRO_FOUND)
    lui_add_demo(lui-demo-cairo)
    target_compile_definitions(lui-demo-cairo PRIVATE LUI_DEMO_CAIRO=1)
    target_link_directories(lui-demo-cairo PRIVATE ${CAIRO_LIBRARY_DIRS})
    target_link_libraries(lui-demo-cairo PRIVATE lui-cairo-${LUI_ABI_VERSION})
endif()

if (APPLE)
    lui_add_demo(lui-demo-cg)
    target_compile_definitions(lui-demo-cg PRIVATE LUI_DEMO_CG=1)
endif()

if(WIN32)
    lui_add_demo(lui-demo-d2d)
    target_compile_definitions(lui-demo-d2d PRIVATE LUI_DEMO_D2D=1)
    lui_add_demo(lui-demo-gdi)
    target_compile_definitions(lui-demo-gdi PRIVATE LUI_DEMO_GDI=1)
endif()
