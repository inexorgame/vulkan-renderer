cmake_minimum_required(VERSION 3.24)
project(inexor-vulkan-renderer CXX)

# Stop in source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Options
option(INEXOR_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(INEXOR_BUILD_DOC "Build documentation" OFF)
option(INEXOR_BUILD_EXAMPLE "Build example" ON)
option(INEXOR_BUILD_TESTS "Build tests" OFF)
option(INEXOR_FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)

message(STATUS "INEXOR_BUILD_BENCHMARKS = ${INEXOR_BUILD_BENCHMARKS}")
message(STATUS "INEXOR_BUILD_DOC = ${INEXOR_BUILD_DOC}")
message(STATUS "INEXOR_BUILD_EXAMPLE = ${INEXOR_BUILD_EXAMPLE}")
message(STATUS "INEXOR_BUILD_TESTS= ${INEXOR_BUILD_TESTS}") 

message(STATUS "CMAKE_VERSION = ${CMAKE_VERSION}")
message(STATUS "CMAKE_GENERATOR = ${CMAKE_GENERATOR}")
message(STATUS "C Compiler executable: ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler executable: ${CMAKE_CXX_COMPILER}")
message(STATUS "Linker executable: ${CMAKE_LINKER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# The marked constants in application.hpp will be replaced with the following values
set(INEXOR_ENGINE_NAME "Inexor Engine")
set(INEXOR_APP_NAME "Inexor Vulkan-renderer example")

set(INEXOR_ENGINE_VERSION_MAJOR 0)
set(INEXOR_ENGINE_VERSION_MINOR 1)
set(INEXOR_ENGINE_VERSION_PATCH 0)

set(INEXOR_APP_VERSION_MAJOR 0)
set(INEXOR_APP_VERSION_MINOR 1)
set(INEXOR_APP_VERSION_PATCH 0)

# Download dependencies through CMake
include(cmake/dependencies.cmake)

# Enable GCC/clang ANSI-colored terminal output using Ninja build tool
# TODO: Switch to `CMAKE_COLOR_DIAGNOSTICS` with cmake 3.24 in the future
if (FORCE_COLORED_OUTPUT)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
endif()

add_subdirectory(shaders)
add_subdirectory(src)

if(INEXOR_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(INEXOR_BUILD_DOC)
    add_subdirectory(documentation)
endif()

if(INEXOR_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

if(INEXOR_BUILD_TESTS)
    add_subdirectory(tests)
endif()
