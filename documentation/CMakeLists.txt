cmake_minimum_required(VERSION 3.19)

option(INEXOR_BUILD_DOCUMENTATION_USE_VENV "Generate Python virtual environment" ON)

find_package(Doxygen REQUIRED)
message(STATUS "Found Doxygen: ${DOXYGEN_EXECUTABLE}")

find_package(Python COMPONENTS Interpreter REQUIRED)
if (INEXOR_BUILD_DOCUMENTATION_USE_VENV)
    set(inexor_venv "${CMAKE_CURRENT_BINARY_DIR}/venv")
    message(STATUS "Creating Python venv at ${inexor_venv}")
    execute_process(COMMAND ${Python_EXECUTABLE} -m venv ${inexor_venv})
    set(ENV{VIRTUAL_ENV} ${inexor_venv})
    set(Python_FIND_VIRTUALENV FIRST)
    unset(Python_EXECUTABLE)
    find_package(Python COMPONENTS Interpreter REQUIRED)

    execute_process(
        COMMAND ${Python_EXECUTABLE} -m pip install --no-cache wheel
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${Python_EXECUTABLE} -m pip install --no-cache -r requirements.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

execute_process(
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../.github/helper/req_check.py requirements.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY
)

find_program(SPHINX_EXECUTABLE sphinx-build HINTS ${inexor_venv}/Scripts ${inexor_venv}/bin REQUIRED)
message(STATUS "Found Sphinx: ${SPHINX_EXECUTABLE}")

# Sphinx cache with pickled ReST documents
set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
add_custom_target(inexor-vulkan-renderer-documentation ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/_doctrees"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/html"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/source/auto-generated"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/source/exhale-generated"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/doxygen-output"
    COMMAND ${SPHINX_EXECUTABLE} -j auto -T -E -a -b html -d "${SPHINX_CACHE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/source" "${CMAKE_CURRENT_BINARY_DIR}/html"
    COMMENT "Building HTML documentation with Sphinx"
)
add_custom_target(inexor-vulkan-renderer-documentation-linkcheck
    COMMAND ${SPHINX_EXECUTABLE} -j auto -T -b linkcheck -d "${SPHINX_CACHE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/source" "${CMAKE_CURRENT_BINARY_DIR}/html"
    COMMENT "Building linkcheck documentation with Sphinx"
)
