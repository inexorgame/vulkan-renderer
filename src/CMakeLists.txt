set(INEXOR_SOURCE_FILES
    vulkan-renderer/input/gamepad_data.cpp
    vulkan-renderer/input/input.cpp
    vulkan-renderer/input/keyboard_mouse_data.cpp

    vulkan-renderer/octree/serialization/byte_stream.cpp
    vulkan-renderer/octree/serialization/nxoc_parser.cpp

    vulkan-renderer/octree/collision_query.cpp
    vulkan-renderer/octree/collision.cpp
    vulkan-renderer/octree/cube.cpp
    vulkan-renderer/octree/indentation.cpp

    vulkan-renderer/render-graph/buffer_copy_batch_builder.cpp
    vulkan-renderer/render-graph/buffer.cpp
    vulkan-renderer/render-graph/frame_sync_manager.cpp
    vulkan-renderer/render-graph/graphics_pass_builder.cpp
    vulkan-renderer/render-graph/graphics_pass.cpp
    vulkan-renderer/render-graph/render_graph.cpp
    vulkan-renderer/render-graph/resource_descriptor_manager.cpp
    vulkan-renderer/render-graph/staging_buffer.cpp
    vulkan-renderer/render-graph/swapchain_manager.cpp
    vulkan-renderer/render-graph/texture_copy_batch_builder.cpp
    vulkan-renderer/render-graph/texture.cpp

    vulkan-renderer/render-modules/imgui/imgui_renderer.cpp

    vulkan-renderer/render-modules/octree/octree_renderer.cpp
    vulkan-renderer/render-modules/octree/octree_vertex.cpp

    vulkan-renderer/tools/allocators/pool_allocator.cpp

    vulkan-renderer/tools/camera.cpp
    vulkan-renderer/tools/device_info.cpp
    vulkan-renderer/tools/enumerate.cpp
    vulkan-renderer/tools/exception.cpp
    vulkan-renderer/tools/file.cpp
    vulkan-renderer/tools/fps_limiter.cpp
    vulkan-renderer/tools/make_info.cpp
    vulkan-renderer/tools/queue_selection.cpp
    vulkan-renderer/tools/random.cpp
    vulkan-renderer/tools/representation.cpp
    vulkan-renderer/tools/time_step.cpp

    vulkan-renderer/wrapper/commands/command_buffer_cache.cpp
    vulkan-renderer/wrapper/commands/command_buffer.cpp
    vulkan-renderer/wrapper/commands/command_pool.cpp

    vulkan-renderer/wrapper/core/debug_callback.cpp
    vulkan-renderer/wrapper/core/device.cpp
    vulkan-renderer/wrapper/core/instance.cpp

    vulkan-renderer/wrapper/descriptors/descriptor_pool_allocator.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_pool.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_set_layout.cpp
    vulkan-renderer/wrapper/descriptors/per_frame_descriptor_sets.cpp
    vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.cpp

    vulkan-renderer/wrapper/images/image_view.cpp
    vulkan-renderer/wrapper/images/image.cpp
    vulkan-renderer/wrapper/images/sampler.cpp

    vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.cpp
    vulkan-renderer/wrapper/pipelines/graphics_pipeline.cpp
    vulkan-renderer/wrapper/pipelines/pipeline_cache.cpp
    vulkan-renderer/wrapper/pipelines/pipeline_layout.cpp

    vulkan-renderer/wrapper/swapchains/swapchain_utils.cpp
    vulkan-renderer/wrapper/swapchains/swapchain.cpp

    vulkan-renderer/wrapper/synchronization/fence.cpp
    vulkan-renderer/wrapper/synchronization/pipeline_barrier_batch_builder.cpp
    vulkan-renderer/wrapper/synchronization/semaphore.cpp

    vulkan-renderer/wrapper/windows/surface.cpp
    vulkan-renderer/wrapper/windows/window.cpp

    vulkan-renderer/wrapper/shader.cpp
)

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

set_target_properties(inexor-vulkan-renderer-core-lib PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(inexor-vulkan-renderer-core-lib PUBLIC
    GLFW_INCLUDE_VULKAN
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    VK_NO_PROTOTYPES # Required by volk metaloader
)

# enable warnings for our own code
target_compile_options(inexor-vulkan-renderer-core-lib PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
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

# replace variables in header file with CMake values
configure_file(
    ${PROJECT_SOURCE_DIR}/include/inexor/vulkan-renderer/meta/meta.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/inexor/vulkan-renderer/meta/meta.hpp
)

target_include_directories(inexor-vulkan-renderer-core-lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    # include configured header file which contains engine metadata
    ${CMAKE_CURRENT_BINARY_DIR}/include/
)

# declare use of dependencies
FetchContent_MakeAvailable(CLI11)
FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(stb)
FetchContent_MakeAvailable(tomlplusplus)
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
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui SYSTEM PUBLIC ${imgui_SOURCE_DIR})

target_link_libraries(inexor-vulkan-renderer-core-lib PUBLIC
    CLI11::CLI11
    fmt::fmt
    glfw
    glm::glm
    imgui
    spdlog::spdlog_header_only
    tinygltf
    tomlplusplus::tomlplusplus
    volk::volk
    Vulkan::Headers
    VulkanMemoryAllocator
)

if(MSVC)
    # group dependencies in Visual Studio
    set(THIRD_PARTY_TARGETS
        CLI11
        fmt
        glfw
        glm
        imgui
        spdlog
        tinygltf
        volk
        VulkanMemoryAllocator
        # also those extra ones
        loader_example
        uninstall
        update_mappings
    )
    # move all dependencies into a subfolder
    set_target_properties(${THIRD_PARTY_TARGETS} PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        FOLDER "Dependencies"
    )
endif()
