# Generate font resources using bin2c.py
set(RES_FONTS
    res/Roboto-Regular.ttf
    res/Roboto-Bold.ttf
    res/Roboto-Light.ttf
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
    button.cpp
    embed.cpp
    entry.cpp
    font.cpp
    graphics.cpp
    image.cpp
    main.cpp
    fitment.cpp
    slider.cpp
    style.cpp
    view.cpp
    widget.cpp
    pugl/src/common.c
    pugl/src/internal.c
)

# Platform-specific sources and options
if(APPLE)
    list(APPEND LIBLUI_SOURCES 
        core_graphics.cpp
        embed_mac.mm
        mac_cg.m
        pugl/src/mac.m)
    set_source_files_properties(
        mac_cg.m
        pugl/src/mac.m
        pugl/src/mac_cairo.m 
        pugl/src/mac_gl.m PROPERTIES
            COMPILE_FLAGS "-Wno-deprecated")
elseif(UNIX AND NOT APPLE)
    list(APPEND LIBLUI_SOURCES 
        embed_x11.cpp
        pugl/src/x11.c)
elseif(WIN32)
    list(APPEND LIBLUI_SOURCES 
        direct2d.cpp
        embed_win32.cpp
        win_direct2d.c
        pugl/src/win.c)
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
    PUBLIC
        $<$<PLATFORM_ID:Windows>:LUI_STATIC>
)

# Include directories
target_include_directories(lui-${LUI_ABI_VERSION} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Include pugl from subproject and nanovg sources
target_include_directories(lui-${LUI_ABI_VERSION} PRIVATE
    ${PROJECT_SOURCE_DIR}/src/pugl/include)

# Link dependencies
target_include_directories(lui-${LUI_ABI_VERSION} PRIVATE)
target_link_directories(lui-${LUI_ABI_VERSION} PRIVATE)
target_link_libraries(lui-${LUI_ABI_VERSION} 
    PRIVATE Threads::Threads)

# X11 support for Linux
if(UNIX AND NOT APPLE)
    target_compile_options(lui-${LUI_ABI_VERSION} PRIVATE -fPIC)
    target_link_libraries(lui-${LUI_ABI_VERSION} 
        PRIVATE 
            X11::X11 
            X11::Xcursor
            X11::Xext
            X11::Xrandr)
    # Xsync is optional, may not be available on all systems
    if(TARGET X11::Xsync)
        target_link_libraries(lui-${LUI_ABI_VERSION} PRIVATE X11::Xsync)
    endif()
endif()

# Apple frameworks
if(APPLE)
    target_link_libraries(lui-${LUI_ABI_VERSION} 
        PUBLIC
            "-framework Foundation"
            "-framework CoreVideo"
            "-framework CoreGraphics"
            "-framework Cocoa"
    )
endif()

# Windows Direct2D libraries
if(WIN32)
    target_link_libraries(lui-${LUI_ABI_VERSION}
        PRIVATE
            uuid
            d2d1
            dwrite
            dwmapi
            gdi32
            user32
    )
endif()

# Install just the main library for now
install(TARGETS lui-${LUI_ABI_VERSION}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Generate and install pkg-config file for main library
configure_file(
    ${CMAKE_SOURCE_DIR}/lui.pc.in
    ${CMAKE_BINARY_DIR}/lui-${LUI_ABI_VERSION}.pc
    @ONLY
)
install(FILES ${CMAKE_BINARY_DIR}/lui-${LUI_ABI_VERSION}.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

function(lui_add_backend tgt)
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
            ${PROJECT_SOURCE_DIR}/src/pugl/include
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
            ${CAIRO_INCLUDE_DIRS}
    )

    target_link_libraries(${tgt} PUBLIC lui-${LUI_ABI_VERSION})

    if(UNIX AND NOT APPLE)
        target_compile_options(${tgt} PRIVATE -fPIC)
    endif()

    install(TARGETS ${tgt}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endfunction()

if(NOT WIN32)
    # OpenGL/NanoVG Backend
    add_library(lui-gl-${LUI_ABI_VERSION}
        nanovg.cpp
        nanovglib.cpp
        opengl.cpp
        ${RES_GENERATED}
    )

    if(APPLE)
        target_sources(lui-gl-${LUI_ABI_VERSION} PRIVATE
            pugl/src/mac_gl.m)
        target_link_libraries(lui-gl-${LUI_ABI_VERSION} 
            PRIVATE "-framework OpenGL")
        target_compile_definitions(lui-gl-${LUI_ABI_VERSION} PRIVATE NANOVG_GL2)
    elseif(UNIX AND NOT APPLE)
        # Linux: use libepoxy for OpenGL function loading
        target_sources(lui-gl-${LUI_ABI_VERSION} PRIVATE
            pugl/src/x11_gl.c)
        if(LIBEPOXY_FOUND)
            target_link_libraries(lui-gl-${LUI_ABI_VERSION} 
                PRIVATE ${LIBEPOXY_LIBRARIES} GL X11)
            target_include_directories(lui-gl-${LUI_ABI_VERSION} 
                PRIVATE ${LIBEPOXY_INCLUDE_DIRS})
            target_compile_definitions(lui-gl-${LUI_ABI_VERSION} PRIVATE NANOVG_GL3)
        else()
            message(FATAL_ERROR "libepoxy not found - required for OpenGL backend on Linux")
        endif()
    endif()

    lui_add_backend(lui-gl-${LUI_ABI_VERSION})
    target_include_directories(lui-gl-${LUI_ABI_VERSION} 
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/nanovg)

    # Generate and install pkg-config file for OpenGL backend
    configure_file(
        ${CMAKE_SOURCE_DIR}/lui-gl.pc.in
        ${CMAKE_BINARY_DIR}/lui-gl-${LUI_ABI_VERSION}.pc
        @ONLY
    )
    install(FILES ${CMAKE_BINARY_DIR}/lui-gl-${LUI_ABI_VERSION}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()

# Cairo Backend
if(CAIRO_FOUND)
    add_library(lui-cairo-${LUI_ABI_VERSION}
        cairo.cpp)
    
    lui_add_backend(lui-cairo-${LUI_ABI_VERSION})
    
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
        target_sources(lui-cairo-${LUI_ABI_VERSION}  PRIVATE pugl/src/mac_cairo.m)
        target_link_libraries(lui-cairo-${LUI_ABI_VERSION} PRIVATE
            "-framework CoreGraphics"
        )
    elseif(UNIX AND NOT APPLE)
        target_sources(lui-cairo-${LUI_ABI_VERSION}  PRIVATE pugl/src/x11_cairo.c)
    elseif(WIN32)
        target_sources(lui-cairo-${LUI_ABI_VERSION}  PRIVATE pugl/src/win_cairo.c)
        target_link_libraries(lui-cairo-${LUI_ABI_VERSION} PRIVATE gdi32)
    endif()
    
    message(STATUS "Cairo backend enabled - building lui-cairo-${LUI_ABI_VERSION}")
    
    # Generate and install pkg-config file for Cairo backend
    configure_file(
        ${PROJECT_SOURCE_DIR}/lui-cairo.pc.in
        ${PROJECT_BINARY_DIR}/lui-cairo-${LUI_ABI_VERSION}.pc
        @ONLY
    )
    install(FILES ${CMAKE_BINARY_DIR}/lui-cairo-${LUI_ABI_VERSION}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
else()
    message(STATUS "Cairo not found - Cairo backend will not be built")
endif()
