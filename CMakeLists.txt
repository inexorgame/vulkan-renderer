cmake_minimum_required(VERSION 3.4)

# stop in source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# options
option(INEXOR_BUILD_DOC "build documentation" OFF)

message(STATUS "INEXOR_BUILD_DOC = ${INEXOR_BUILD_DOC}")

project(inexor-vulkan-renderer)

message(STATUS "CMAKE_VERSION = ${CMAKE_VERSION}")
message(STATUS "CMAKE_GENERATOR = ${CMAKE_GENERATOR}")
message(STATUS "C Compiler executable: ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler executable: ${CMAKE_CXX_COMPILER}")
message(STATUS "Linker executable: ${CMAKE_LINKER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

file(
    GLOB_RECURSE source_files
    "src/*.hpp"
    "src/*.cpp"
    "tests/*.*"
    "benchmarks/*.*"
)

# Use the folder structure in source code directory as project structure in Visual Studio.
function(assign_source_group)
    foreach (source_files IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${source_files}")
            file(RELATIVE_PATH _source_rel "${PROJECT_SOURCE_DIR}" "${source_files}")
        else ()
            set(_source_rel "${source_files}")
        endif ()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${source_files}")
    endforeach ()
endfunction(assign_source_group)

set(NORMAL_SOURCE_FILES)
set(TEST_SOURCE_FILES)
set(BENCHMARK_SOURCE_FILES)

# Sort source files by tests and benchmarks.
foreach (file ${source_files})
    if (file MATCHES ".*_test.*")
        set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} ${file})
    elseif (file MATCHES ".*_benchmark.*")
        set(BENCHMARK_SOURCE_FILES ${BENCHMARK_SOURCE_FILES} ${file})
    else ()
        set(NORMAL_SOURCE_FILES ${NORMAL_SOURCE_FILES} ${file})
    endif ()
endforeach ()

# Use the folder structure in source code directory as project structure in Visual Studio.
assign_source_group(${NORMAL_SOURCE_FILES})
assign_source_group(${TEST_SOURCE_FILES})
assign_source_group(${BENCHMARK_SOURCE_FILES})

if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
    message(STATUS "Using module to find Vulkan")
    find_package(Vulkan REQUIRED)
endif ()

# dependency setup with conan
include(${PROJECT_SOURCE_DIR}/cmake/conan_setup.cmake)

add_executable(
    inexor-vulkan-renderer

    src/main.cpp
    ${NORMAL_SOURCE_FILES}
)
target_include_directories(
    inexor-vulkan-renderer PRIVATE
    Vulkan::Vulkan
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/third_party
)
target_link_libraries(
    inexor-vulkan-renderer
    ${CONAN_LIBS}
    Vulkan::Vulkan
)
target_compile_features(inexor-vulkan-renderer PRIVATE cxx_std_17)
# Use multiple threads to generate code.
if (MSVC)
    target_compile_options(inexor-vulkan-renderer PRIVATE "/MP")
endif ()

# Use root folder of the repository as working directory in Visual Studio debugger!
# Otherwise Visual Studio can't find any files like shaders or textures.
if (WIN32)
    set_property(TARGET inexor-vulkan-renderer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif ()

file(
    GLOB_RECURSE NORMAL_SOURCE_FILES_WITHOUT_MAIN
    "src/vulkan-renderer/*"
)

add_executable(
    inexor-vulkan-renderer-tests

    ${NORMAL_SOURCE_FILES_WITHOUT_MAIN}
    ${TEST_SOURCE_FILES}
)
target_include_directories(inexor-vulkan-renderer-tests PRIVATE Vulkan::Vulkan)
target_link_libraries(
    inexor-vulkan-renderer-tests
    ${CONAN_LIBS}
    Vulkan::Vulkan
)
target_compile_features(inexor-vulkan-renderer-tests PRIVATE cxx_std_17)
if (MSVC)
    # Use multiple threads to compile the code.
    target_compile_options(inexor-vulkan-renderer-tests PRIVATE "/MP")
endif ()

# Use root folder of the repository as working directory in Visual Studio debugger!
# Otherwise Visual Studio can't find any files like shaders or textures.
if (WIN32)
    set_property(TARGET inexor-vulkan-renderer-tests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif ()

add_executable(
    inexor-vulkan-renderer-benchmarks

    ${NORMAL_SOURCE_FILES_WITHOUT_MAIN}
    ${BENCHMARK_SOURCE_FILES}
)
target_include_directories(inexor-vulkan-renderer-benchmarks PRIVATE Vulkan::Vulkan)
target_link_libraries(
    inexor-vulkan-renderer-benchmarks
    ${CONAN_LIBS}
    Vulkan::Vulkan
)
target_compile_features(inexor-vulkan-renderer-benchmarks PRIVATE cxx_std_17)
# Use multiple threads to compile the code.
if (MSVC)
    target_compile_options(inexor-vulkan-renderer-benchmarks PRIVATE "/MP")
endif ()

if (WIN32)
    set_property(TARGET inexor-vulkan-renderer-benchmarks PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif ()

# documentation
if (${INEXOR_BUILD_DOC})
    add_subdirectory(${PROJECT_SOURCE_DIR}/documentation)
endif ()