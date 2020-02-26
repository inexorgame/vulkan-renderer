cmake_minimum_required(VERSION 3.4)

project(inexor-vulkan-renderer)

file(GLOB_RECURSE source_files
  "src/*.hpp"
  "src/*.cpp"
)

# Use the folder structure in source code directory as project structure in Visual Studio.
function(assign_source_group)
    foreach(source_files IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${source_files}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${source_files}")
        else()
            set(_source_rel "${source_files}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${source_files}")
    endforeach()
endfunction(assign_source_group)


if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Using module to find Vulkan")
	find_package(Vulkan)
endif()

# Dependency setup via conan.
# Download conan executer in case it does not exists.
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.14/conan.cmake"
                 "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
endif()

# Execute conan build instructions.
include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

conan_cmake_run(CONANFILE conanfile.py
                BASIC_SETUP
                BUILD outdated
                PROFILE default
                PROFILE_AUTO build_type
                KEEP_RPATHS
)

# Use the folder structure in source code directory as project structure in Visual Studio.
assign_source_group(${source_files})

add_executable(inexor-vulkan-renderer src/main.cpp ${source_files})

conan_target_link_libraries(inexor-vulkan-renderer PUBLIC ${Vulkan_LIBS} ${CONAN_LIBS})

# Use C++17!
target_compile_features(inexor-vulkan-renderer PRIVATE cxx_std_17)

IF(WIN32)
	target_compile_definitions(inexor-vulkan-renderer PRIVATE VK_USE_PLATFORM_WIN32_KHR)
ENDIF()

target_include_directories(inexor-vulkan-renderer PRIVATE Vulkan::Vulkan)

target_link_libraries(inexor-vulkan-renderer Vulkan::Vulkan)
