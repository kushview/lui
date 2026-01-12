set(LUI_LUA_VERSION "${LUA_VERSION}" CACHE STRING "Lua version to use")

add_library (lui-luamod MODULE
    lui.cpp)

set_target_properties(lui-luamod PROPERTIES
    OUTPUT_NAME "lui"
    PREFIX ""
    SUFFIX ".so"
)

if(APPLE)
    set_target_properties(lui-luamod PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
elseif(UNIX)
    set_target_properties(lui-luamod PROPERTIES
        LINK_FLAGS "-Wl,--unresolved-symbols=ignore-all"
    )
endif()

if(WIN32)
    set_target_properties(lui-luamod PROPERTIES SUFFIX ".dll")
endif()

target_include_directories(lui-luamod PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/${LUI_LUA_VERSION}
    ${PROJECT_SOURCE_DIR}/include)
target_link_directories(lui-luamod PRIVATE 
    ${CAIRO_LIBRARY_DIRS})
target_link_libraries(lui-luamod PRIVATE
    lui-cairo-${LUI_ABI_VERSION})
