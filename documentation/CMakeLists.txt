# https://eb2.co/blog/2012/03/sphinx-and-cmake-beautiful-documentation-for-c---projects/
cmake_minimum_required(VERSION 3.12)

find_package(Doxygen)
if (NOT Doxygen_FOUND)
    message(FATAL_ERROR "Doxygen not found")
endif ()
message(STATUS "Found Doxygen: ${DOXYGEN_EXECUTABLE}")

find_package(Sphinx REQUIRED PATHS ${CMAKE_CURRENT_SOURCE_DIR}/cmake NO_DEFAULT_PATH)
if (NOT Sphinx_FOUND)
    message(FATAL_ERROR "Sphinx not found")
endif ()
message(STATUS "Found Sphinx: ${SPHINX_EXECUTABLE}")

# Sphinx cache with pickled ReST documents
set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
add_custom_target(
    inexor-vulkan-renderer-documentation ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/_doctrees"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/html"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/source/exhale-generated"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/doxygen-output"
    COMMAND ${SPHINX_EXECUTABLE} -j auto -T -E -a -b html -d "${SPHINX_CACHE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/source" "${CMAKE_CURRENT_BINARY_DIR}/html"
    COMMENT "Building HTML documentation with Sphinx"
)
