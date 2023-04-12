cmake_minimum_required(VERSION 3.16)
project(inexor-vulkan-renderer CXX)

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

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 8.0.1
    GIT_SHALLOW ON
    GIT_PROGRESS ON)
FetchContent_MakeAvailable(fmt)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.2
    GIT_SHALLOW ON
    GIT_PROGRESS ON)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
    GIT_SHALLOW ON
    GIT_PROGRESS ON)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.85
    GIT_SHALLOW ON
    GIT_PROGRESS ON)
FetchContent_MakeAvailable(imgui)
add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.9.2
    GIT_SHALLOW ON
    GIT_PROGRESS ON)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG af1a5bc352164740c1cc1354942b1c6b72eacb8a
    GIT_PROGRESS ON)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG 81f7dbe53a112d05217a79bb1c986f9ada3b6631
    GIT_PROGRESS ON)
FetchContent_MakeAvailable(tinygltf)

FetchContent_Declare(toml
    GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
    GIT_TAG v3.7.0
    GIT_SHALLOW ON
    GIT_PROGRESS ON)
FetchContent_MakeAvailable(toml)

FetchContent_Declare(vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.0.1
    GIT_PROGRESS ON)
FetchContent_MakeAvailable(vma)

# Extract the current git sha
find_package(Git REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --abbrev=7
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE INEXOR_GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

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
