set(INEXOR_SOURCE_FILES
    vulkan-renderer/application.cpp
    vulkan-renderer/exception.cpp
    vulkan-renderer/imgui.cpp
    vulkan-renderer/render_graph.cpp
    vulkan-renderer/renderer.cpp

    vulkan-renderer/input/gamepad_data.cpp
    vulkan-renderer/input/input.cpp
    vulkan-renderer/input/keyboard_mouse_data.cpp

    vulkan-renderer/io/byte_stream.cpp
    vulkan-renderer/io/nxoc_parser.cpp

    vulkan-renderer/tools/camera.cpp
    vulkan-renderer/tools/cla_parser.cpp
    vulkan-renderer/tools/device_info.cpp
    vulkan-renderer/tools/enumerate.cpp
    vulkan-renderer/tools/file.cpp
    vulkan-renderer/tools/fps_counter.cpp
    vulkan-renderer/tools/representation.cpp
    vulkan-renderer/tools/time_step.cpp

    vulkan-renderer/wrapper/cpu_texture.cpp
    vulkan-renderer/wrapper/device.cpp
    vulkan-renderer/wrapper/framebuffer.cpp
    vulkan-renderer/wrapper/gpu_memory_buffer.cpp
    vulkan-renderer/wrapper/gpu_texture.cpp
    vulkan-renderer/wrapper/image.cpp
    vulkan-renderer/wrapper/instance.cpp
    vulkan-renderer/wrapper/make_info.cpp
    vulkan-renderer/wrapper/shader.cpp
    vulkan-renderer/wrapper/swapchain.cpp
    vulkan-renderer/wrapper/uniform_buffer.cpp

    vulkan-renderer/wrapper/commands/command_buffer.cpp
    vulkan-renderer/wrapper/commands/command_pool.cpp

    vulkan-renderer/wrapper/descriptors/descriptor.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_builder.cpp

    vulkan-renderer/wrapper/window/window.cpp
    vulkan-renderer/wrapper/window/surface.cpp

    vulkan-renderer/wrapper/synchronization/fence.cpp
    vulkan-renderer/wrapper/synchronization/semaphore.cpp

    vulkan-renderer/octree/collision.cpp
    vulkan-renderer/octree/collision_query.cpp
    vulkan-renderer/octree/cube.cpp
    vulkan-renderer/octree/indentation.cpp)

foreach(FILE ${INEXOR_SOURCE_FILES})
    get_filename_component(PARENT_DIR "${FILE}" PATH)

    string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")

    if("${FILE}" MATCHES ".*\\.cpp")
        set(GROUP "Source Files\\${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.hpp")
        set(GROUP "Header Files\\${GROUP}")
    endif()

    source_group("${GROUP}" FILES "${FILE}")
endforeach()

add_library(inexor-vulkan-renderer-core-lib ${INEXOR_SOURCE_FILES})

add_dependencies(inexor-vulkan-renderer-core-lib inexor-shaders)

set_target_properties(
    inexor-vulkan-renderer-core-lib PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(
    inexor-vulkan-renderer-core-lib

    PUBLIC
    GLFW_INCLUDE_VULKAN
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    VK_NO_PROTOTYPES # Required by volk metaloader
)

# enable multi processor building if VS project
if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
    target_compile_options(inexor-vulkan-renderer-core-lib PRIVATE "/MP")
endif()

# enable exceptions when using MSVC toolchain, makes Clang on windows possible
# enable correct reporting of the __cplusplus macro so correct code paths are selected
if(MSVC)
    target_compile_options(inexor-vulkan-renderer-core-lib PRIVATE "-EHs")
    target_compile_options(inexor-vulkan-renderer-core-lib PRIVATE "/Zc:__cplusplus")
endif()

# Replace variables in header file with CMake values
configure_file(
    ${PROJECT_SOURCE_DIR}/include/inexor/vulkan-renderer/meta/meta.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/inexor/vulkan-renderer/meta/meta.hpp
)

target_include_directories(
    inexor-vulkan-renderer-core-lib

    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    # Include configured header file which contains engine metadata
    ${CMAKE_CURRENT_BINARY_DIR}/include/
)

# Declare use of dependencies
FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(stb)
FetchContent_MakeAvailable(toml)
FetchContent_MakeAvailable(Vulkan)
FetchContent_MakeAvailable(volk)
FetchContent_MakeAvailable(vma)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(tinygltf)
FetchContent_MakeAvailable(imgui)
add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

target_link_libraries(inexor-vulkan-renderer-core-lib
    PUBLIC
    fmt::fmt
    glfw
    glm::glm
    imgui
    spdlog::spdlog_header_only
    tinygltf
    toml11::toml11
    volk::volk
    Vulkan::Headers
    VulkanMemoryAllocator)

if(MSVC)
    # Group dependencies in Visual Studio
    set(THIRD_PARTY_TARGETS
        glfw
        fmt
        glm
        imgui
        spdlog
        volk
        VulkanMemoryAllocator
        tinygltf
        toml11
        # Also those extra ones
        loader_example
        uninstall
        update_mappings
        Continuous
        Experimental
        Nightly
        NightlyMemoryCheck
    )
    # Move all dependencies into a subfolder
    set_target_properties(${THIRD_PARTY_TARGETS}
        PROPERTIES
            EXCLUDE_FROM_ALL TRUE
            FOLDER "Dependencies"
    )
endif()
