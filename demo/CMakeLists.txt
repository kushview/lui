# Demo resources
set(DEMO_RES_FONTS
    res/lv2_logo.png
)

set(DEMO_RES_GENERATED)
foreach(resource ${DEMO_RES_FONTS})
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
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CAIRO_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
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

target_include_directories(lui-demo-exe PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS}
)

target_compile_definitions(lui-demo-exe PRIVATE
    LUI_NO_SYMBOL_EXPORT
    LUI_DEMO_OPENGL=1
)

target_link_libraries(lui-demo-exe PRIVATE
    lui-demo
)

# macOS app bundle
if(APPLE)
    add_custom_command(TARGET lui-demo-exe POST_BUILD
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/bundle.py
                --binary $<TARGET_FILE:lui-demo-exe>
                --output ${CMAKE_BINARY_DIR}/LUI.app
                --plist ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
        COMMENT "Creating LUI.app bundle"
    )
endif()

# Install
install(TARGETS lui-demo-exe
    RUNTIME DESTINATION bin
)
