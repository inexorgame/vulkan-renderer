cmake_minimum_required(VERSION 3.9)

# https://cmake.org/cmake/help/latest/prop_tgt/MSVC_RUNTIME_LIBRARY.html
cmake_policy(SET CMP0091 NEW)

project(inexor-vulkan-renderer)

# Stop in source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Options
option(INEXOR_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(INEXOR_BUILD_DOC "Build documentation" OFF)
option(INEXOR_BUILD_EXAMPLE "Build example" ON)
option(INEXOR_BUILD_TESTS "Build tests" OFF)
set(INEXOR_CONAN_PROFILE "default" CACHE STRING "conan profile")
option(INEXOR_USE_VMA_RECORDING "Use VulkanMemoryAllocator recording feature" OFF)

message(STATUS "INEXOR_BUILD_BENCHMARKS = ${INEXOR_BUILD_BENCHMARKS}")
message(STATUS "INEXOR_BUILD_DOC = ${INEXOR_BUILD_DOC}")
message(STATUS "INEXOR_BUILD_EXAMPLE = ${INEXOR_BUILD_EXAMPLE}")
message(STATUS "INEXOR_BUILD_TESTS= ${INEXOR_BUILD_TESTS}")
message(STATUS "INEXOR_CONAN_PROFILE = ${INEXOR_CONAN_PROFILE}")
message(STATUS "INEXOR_USE_VMA_RECORDING = ${INEXOR_USE_VMA_RECORDING}")

message(STATUS "CMAKE_VERSION = ${CMAKE_VERSION}")
message(STATUS "CMAKE_GENERATOR = ${CMAKE_GENERATOR}")
message(STATUS "C Compiler executable: ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler executable: ${CMAKE_CXX_COMPILER}")
message(STATUS "Linker executable: ${CMAKE_LINKER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

find_package(Vulkan REQUIRED)

# Dependency setup with conan
include(conan_setup)

# The marked constants in application.hpp will be replaced with the following values
set(INEXOR_ENGINE_NAME "Inexor Engine")
set(INEXOR_APP_NAME "Inexor Vulkan-renderer example")

set(INEXOR_ENGINE_VERSION_MAJOR 0)
set(INEXOR_ENGINE_VERSION_MINOR 1)
set(INEXOR_ENGINE_VERSION_PATCH 0)

set(INEXOR_APP_VERSION_MAJOR 0)
set(INEXOR_APP_VERSION_MINOR 1)
set(INEXOR_APP_VERSION_PATCH 0)

find_package(Git REQUIRED)

# Extract the current git sha
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --abbrev=7
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE INEXOR_GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_subdirectory(shaders)

if(MSVC)
    add_subdirectory(platform/windows)
endif()

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
