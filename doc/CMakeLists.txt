# Documentation build configuration

find_package(Doxygen)
# find_package(Sphinx)

if(DOXYGEN_FOUND)
    # Configure Doxyfile
    set(LUI_SRCDIR ${CMAKE_SOURCE_DIR})
    set(DOX_OUTPUT ${CMAKE_BINARY_DIR}/doc)
    
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_BINARY_DIR}/Doxyfile
        @ONLY
    )
    
    # Add doxygen target
    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    
    message(STATUS "Doxygen found - 'make doxygen' target available")
else()
    message(STATUS "Doxygen not found - API documentation will not be built")
endif()

# Sphinx documentation
find_program(SPHINX_EXECUTABLE 
    NAMES sphinx-build
    DOC "Path to sphinx-build executable")

if(SPHINX_EXECUTABLE)
    # Configure conf.py
    set(LUI_VERSION ${PROJECT_VERSION})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in
        ${CMAKE_BINARY_DIR}/doc/conf.py
        @ONLY
    )
    
    # Configure overview.rst if it exists
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/overview.rst.in)
        set(lui_abi_version "0") # Set appropriately
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/overview.rst.in
            ${CMAKE_BINARY_DIR}/doc/overview.rst
            @ONLY
        )
    endif()
    
    # Copy RST files to build directory
    file(GLOB RST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.rst)
    foreach(rst_file ${RST_FILES})
        get_filename_component(rst_name ${rst_file} NAME)
        configure_file(
            ${rst_file}
            ${CMAKE_BINARY_DIR}/doc/${rst_name}
            COPYONLY
        )
    endforeach()
    
    # Copy subdirectories
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/api)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/api 
             DESTINATION ${CMAKE_BINARY_DIR}/doc)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ui 
             DESTINATION ${CMAKE_BINARY_DIR}/doc)
    endif()
    
    # Add sphinx target
    add_custom_target(sphinx
        COMMAND ${SPHINX_EXECUTABLE}
            -b html
            -c ${CMAKE_BINARY_DIR}/doc
            ${CMAKE_BINARY_DIR}/doc
            ${CMAKE_BINARY_DIR}/doc/html
        DEPENDS doxygen
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating documentation with Sphinx"
        VERBATIM
    )
    
    message(STATUS "Sphinx found - 'make sphinx' target available")
else()
    message(STATUS "Sphinx not found - HTML documentation will not be built")
endif()

# Combined docs target
if(DOXYGEN_FOUND AND SPHINX_EXECUTABLE)
    add_custom_target(docs
        DEPENDS doxygen sphinx
        COMMENT "Building all documentation"
    )
    message(STATUS "Documentation targets available: doxygen, sphinx, docs")
endif()
