# Generate font resources using bin2c.py
set(RES_FONTS
    ui/res/Roboto-Regular.ttf
    ui/res/Roboto-Bold.ttf
    ui/res/Roboto-Light.ttf
)

set(RES_GENERATED)
foreach(font ${RES_FONTS})
    get_filename_component(font_name ${font} NAME_WE)
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${font_name}.h")
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/scripts/bin2c.py"
                ${CMAKE_CURRENT_SOURCE_DIR}/${font} ${output} ${CMAKE_CURRENT_SOURCE_DIR}/${font}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${font}
        COMMENT "Generating ${font_name}.h"
    )
    list(APPEND RES_GENERATED ${output})
endforeach()

# Main library sources
set(LIBLUI_SOURCES
    ui/button.cpp
    ui/embed.cpp
    ui/entry.cpp
    ui/font.cpp
    ui/graphics.cpp
    ui/image.cpp
    ui/main.cpp
    ui/fitment.cpp
    ui/slider.cpp
    ui/style.cpp
    ui/view.cpp
    ui/widget.cpp
    "ui/pugl/src/common.c"
    "ui/pugl/src/internal.c"
)

# Platform-specific sources and options
if(APPLE)
    list(APPEND LIBLUI_SOURCES 
        ui/embed_mac.mm
        ui/pugl/src/mac.m)
    set_source_files_properties(
        ui/pugl/src/mac.m
        ui/pugl/src/mac_cairo.m 
        ui/pugl/src/mac_gl.m PROPERTIES
            COMPILE_FLAGS "-Wno-deprecated")
elseif(UNIX AND NOT APPLE)
    list(APPEND LIBLUI_SOURCES 
        ui/embed_x11.cpp)
elseif(WIN32)
    list(APPEND LIBLUI_SOURCES 
        ui/embed_win32.cpp)
endif()

# Create the main library
add_library(lui-${LUI_ABI_VERSION}
    ${LIBLUI_SOURCES}
)

# Set properties
set_target_properties(lui-${LUI_ABI_VERSION} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${LUI_ABI_VERSION}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Add compile definitions
target_compile_definitions(lui-${LUI_ABI_VERSION}
    PRIVATE
        LUI_BUILD
        PUGL_STATIC
)

# Include directories
target_include_directories(lui-${LUI_ABI_VERSION} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Include pugl from subproject and nanovg sources
target_include_directories(lui-${LUI_ABI_VERSION} PRIVATE
    ${PROJECT_SOURCE_DIR}/src/ui/pugl/include)

# Link dependencies
target_include_directories(lui-${LUI_ABI_VERSION} PRIVATE)
target_link_directories(lui-${LUI_ABI_VERSION} PRIVATE)
target_link_libraries(lui-${LUI_ABI_VERSION} 
    PRIVATE Threads::Threads)

# Apple frameworks
if(APPLE)
    target_link_libraries(lui-${LUI_ABI_VERSION} PRIVATE
        "-framework Foundation"
        "-framework CoreVideo"
        "-framework CoreGraphics"
        "-framework Cocoa"
        "-framework OpenGL"
    )
endif()

# Install just the main library for now
install(TARGETS lui-${LUI_ABI_VERSION}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

function(lui_setup_backend_library tgt)
    set_target_properties(${tgt} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${LUI_ABI_VERSION}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    target_compile_definitions(${tgt} PRIVATE 
        PUGL_STATIC 
        LUI_BUILD
    )

    target_include_directories(${tgt}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src/ui/pugl/include
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
            ${CAIRO_INCLUDE_DIRS}
    )

    install(TARGETS ${tgt}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endfunction()

# OpenGL/NanoVG Backend
add_library(lui-gl-${LUI_ABI_VERSION}
    ui/nanovg.cpp
    ui/nanovglib.cpp
    ui/opengl.cpp
    ${RES_GENERATED}
)

if(APPLE)
    target_sources(lui-gl-${LUI_ABI_VERSION} PRIVATE
        ui/pugl/src/mac_gl.m)
endif()

lui_setup_backend_library(lui-gl-${LUI_ABI_VERSION})
target_include_directories(lui-gl-${LUI_ABI_VERSION} 
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/nanovg)
target_compile_definitions(lui-gl-${LUI_ABI_VERSION} PRIVATE NANOVG_GL2)

# Cairo Backend
if(CAIRO_FOUND)
    add_library(lui-cairo-${LUI_ABI_VERSION}
        ui/cairo.cpp
        ui/pugl/src/mac_cairo.m)
    
    lui_setup_backend_library(lui-cairo-${LUI_ABI_VERSION})
    
    target_include_directories(lui-cairo-${LUI_ABI_VERSION} 
        PRIVATE ${CAIRO_INCLUDE_DIRS})
    target_link_directories(lui-cairo-${LUI_ABI_VERSION}
        PRIVATE ${CAIRO_LIBRARY_DIRS})
    target_link_libraries(lui-cairo-${LUI_ABI_VERSION}
        PUBLIC lui-${LUI_ABI_VERSION})
    target_link_libraries(lui-cairo-${LUI_ABI_VERSION}
        PRIVATE ${CAIRO_LIBRARIES})
    
    # Platform-specific Cairo libraries
    if(APPLE)
        target_link_libraries(lui-cairo-${LUI_ABI_VERSION} PRIVATE
            "-framework CoreGraphics"
        )
    elseif(WIN32)
        target_link_libraries(lui-cairo-${LUI_ABI_VERSION} PRIVATE
            gdi32
        )
    endif()
    
    message(STATUS "Cairo backend enabled - building lui-cairo-${LUI_ABI_VERSION}")
else()
    message(STATUS "Cairo not found - Cairo backend will not be built")
endif()
