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
    ui/pugl.cpp
    ui/button.cpp
    ui/embed.cpp
    ui/entry.cpp
    ui/font.cpp
    ui/graphics.cpp
    ui/image.cpp
    ui/main.cpp
    ui/nanovg.cpp
    ui/nanovglib.cpp
    ui/opengl.cpp
    ui/fitment.cpp
    ui/slider.cpp
    ui/style.cpp
    ui/view.cpp
    ui/widget.cpp
)

# Platform-specific sources
if(APPLE)
    list(APPEND LIBLUI_SOURCES ui/pugl.mm ui/embed_mac.mm)
elseif(UNIX AND NOT APPLE)
    list(APPEND LIBLUI_SOURCES ui/embed_x11.cpp)
elseif(WIN32)
    list(APPEND LIBLUI_SOURCES ui/embed_win32.cpp)
endif()

# Create the main library
add_library(lui-${LUI_ABI_VERSION}
    ${LIBLUI_SOURCES}
    ${RES_GENERATED}
)

# Set properties
set_target_properties(lui-${LUI_ABI_VERSION} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${LUI_ABI_VERSION}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Add compile definitions
target_compile_definitions(lui-${LUI_ABI_VERSION} PRIVATE
    LUI_BUILD
    PUGL_STATIC
    NANOVG_GL2
)

# Include directories
target_include_directories(lui-${LUI_ABI_VERSION} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Include pugl from subproject and nanovg sources
target_include_directories(lui-${LUI_ABI_VERSION} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/ui/pugl/include
    ${CMAKE_CURRENT_SOURCE_DIR}/nanovg
)

# Link dependencies
target_include_directories(lui-${LUI_ABI_VERSION} PRIVATE)

target_link_directories(lui-${LUI_ABI_VERSION} PRIVATE)

target_link_libraries(lui-${LUI_ABI_VERSION} PRIVATE
    Threads::Threads
)

# Add Boost if found
if(Boost_FOUND OR BOOST_FOUND)
    if(Boost_FOUND)
        target_include_directories(lui-${LUI_ABI_VERSION} PRIVATE ${Boost_INCLUDE_DIRS})
        target_link_libraries(lui-${LUI_ABI_VERSION} PRIVATE Boost::boost)
    else()
        target_include_directories(lui-${LUI_ABI_VERSION} PRIVATE ${BOOST_INCLUDE_DIRS})
        target_link_directories(lui-${LUI_ABI_VERSION} PRIVATE ${BOOST_LIBRARY_DIRS})
        target_link_libraries(lui-${LUI_ABI_VERSION} PRIVATE ${BOOST_LIBRARIES})
    endif()
endif()

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

# Cairo support
# Disabled for now - cairo not available
# add_library(lui-cairo-${LUI_ABI_VERSION}
#     ui/cairo.cpp
# )
#
# set_target_properties(lui-cairo-${LUI_ABI_VERSION} PROPERTIES
#     VERSION ${PROJECT_VERSION}
#     SOVERSION ${LUI_ABI_VERSION}
#     CXX_VISIBILITY_PRESET hidden
#     VISIBILITY_INLINES_HIDDEN ON
# )
#
# target_compile_definitions(lui-cairo-${LUI_ABI_VERSION} PRIVATE
#     LUI_BUILD
# )
#
# target_include_directories(lui-cairo-${LUI_ABI_VERSION} PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
#     $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
#     $<INSTALL_INTERFACE:include>
# )
#
# target_include_directories(lui-cairo-${LUI_ABI_VERSION} PRIVATE
#     ${CAIRO_INCLUDE_DIRS}
#     ${PUGL_CAIRO_INCLUDE_DIRS}
# )
#
# target_link_directories(lui-cairo-${LUI_ABI_VERSION} PRIVATE
#     ${CAIRO_LIBRARY_DIRS}
#     ${PUGL_CAIRO_LIBRARY_DIRS}
# )
#
# target_link_libraries(lui-cairo-${LUI_ABI_VERSION} PUBLIC
#     lui-${LUI_ABI_VERSION}
#     ${CAIRO_LIBRARIES}
#     ${PUGL_CAIRO_LIBRARIES}
# )
#
# # Install targets
# install(TARGETS lui-${LUI_ABI_VERSION} lui-cairo-${LUI_ABI_VERSION}
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
# )

# Install just the main library for now
install(TARGETS lui-${LUI_ABI_VERSION}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
