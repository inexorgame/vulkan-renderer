cmake_minimum_required(VERSION 3.4)

project(inexor-vulkan-renderer)

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
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${source_files}")
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

if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
    message(STATUS "Using module to find Vulkan")
    find_package(Vulkan)
endif ()

# Dependency setup via conan.
# Download conan executer in case it does not exists.
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(
        DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.14/conan.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
    )
endif ()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

conan_check(VERSION 1.19.1 REQUIRED)

conan_cmake_run(
    CONANFILE conanfile.py
    BASIC_SETUP
    BUILD outdated
    PROFILE default
    PROFILE_AUTO build_type
    KEEP_RPATHS
)

# Use the folder structure in source code directory as project structure in Visual Studio.
assign_source_group(${NORMAL_SOURCE_FILES})
assign_source_group(${TEST_SOURCE_FILES})
assign_source_group(${BENCHMARK_SOURCE_FILES})

add_executable(inexor-vulkan-renderer src/main.cpp ${NORMAL_SOURCE_FILES})
target_include_directories(
    inexor-vulkan-renderer PRIVATE
    Vulkan::Vulkan
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/third_party
)
conan_target_link_libraries(inexor-vulkan-renderer)
target_compile_features(inexor-vulkan-renderer PRIVATE cxx_std_17)
target_link_libraries(inexor-vulkan-renderer Vulkan::Vulkan)

# Use multiple threads to generate code.
if (MSVC)
	target_compile_options(inexor-vulkan-renderer PRIVATE "/MP")
endif ()

# Use root folder of the repository as working directory in Visual Studio debugger!
# Otherwise Visual Studio can't find any files like shaders or textures.
if (WIN32)
    set_property(TARGET inexor-vulkan-renderer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif ()

file(
    GLOB_RECURSE NORMAL_SOURCE_FILES_WITHOUT_MAIN
    "src/vulkan-renderer/*"
)

add_executable(inexor-vulkan-renderer-tests ${NORMAL_SOURCE_FILES_WITHOUT_MAIN} ${TEST_SOURCE_FILES})
conan_target_link_libraries(inexor-vulkan-renderer-tests)
target_compile_features(inexor-vulkan-renderer-tests PRIVATE cxx_std_17)
target_include_directories(inexor-vulkan-renderer-tests PRIVATE Vulkan::Vulkan)
target_link_libraries(inexor-vulkan-renderer-tests Vulkan::Vulkan)

# Use root folder of the repository as working directory in Visual Studio debugger!
# Otherwise Visual Studio can't find any files like shaders or textures.
if (WIN32)
    set_property(TARGET inexor-vulkan-renderer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif ()

if (MSVC)
	# Use multiple threads to compile the code.
    target_compile_options(inexor-vulkan-renderer-tests PRIVATE "/MP")
endif ()

add_executable(inexor-vulkan-renderer-benchmarks ${NORMAL_SOURCE_FILES_WITHOUT_MAIN} ${BENCHMARK_SOURCE_FILES})
conan_target_link_libraries(inexor-vulkan-renderer-benchmarks)
target_compile_features(inexor-vulkan-renderer-benchmarks PRIVATE cxx_std_17)
target_include_directories(inexor-vulkan-renderer-benchmarks PRIVATE Vulkan::Vulkan)
target_link_libraries(inexor-vulkan-renderer-benchmarks Vulkan::Vulkan)

if (WIN32)
    set_property(TARGET inexor-vulkan-renderer-benchmarks PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif ()

# Use multiple threads to compile the code.
if (MSVC)
    target_compile_options(inexor-vulkan-renderer-benchmarks PRIVATE "/MP")
endif ()
