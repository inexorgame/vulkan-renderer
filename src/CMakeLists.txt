set(INEXOR_SOURCE_FILES
    vulkan-renderer/application.cpp
    vulkan-renderer/availability_checks.cpp
    vulkan-renderer/bezier_curve.cpp
    vulkan-renderer/camera.cpp
    vulkan-renderer/debug_callback.cpp
    vulkan-renderer/fps_counter.cpp
    vulkan-renderer/frame_graph.cpp
    vulkan-renderer/gpu_info.cpp
    vulkan-renderer/imgui.cpp
    vulkan-renderer/renderer.cpp
    vulkan-renderer/settings_decision_maker.cpp
    vulkan-renderer/time_step.cpp

    vulkan-renderer/io/byte_stream.cpp
    vulkan-renderer/io/octree_parser.cpp

    vulkan-renderer/tools/cla_parser.cpp
    vulkan-renderer/tools/file.cpp

    vulkan-renderer/wrapper/command_buffer.cpp
    vulkan-renderer/wrapper/command_pool.cpp
    vulkan-renderer/wrapper/cpu_texture.cpp
    vulkan-renderer/wrapper/device.cpp
    vulkan-renderer/wrapper/fence.cpp
    vulkan-renderer/wrapper/framebuffer.cpp
    vulkan-renderer/wrapper/glfw_context.cpp
    vulkan-renderer/wrapper/gpu_memory_buffer.cpp
    vulkan-renderer/wrapper/gpu_texture.cpp
    vulkan-renderer/wrapper/graphics_pipeline.cpp
    vulkan-renderer/wrapper/image.cpp
    vulkan-renderer/wrapper/instance.cpp
    vulkan-renderer/wrapper/make_info.cpp
    vulkan-renderer/wrapper/mesh_buffer.cpp
    vulkan-renderer/wrapper/once_command_buffer.cpp
    vulkan-renderer/wrapper/renderpass.cpp
    vulkan-renderer/wrapper/resource_descriptor.cpp
    vulkan-renderer/wrapper/semaphore.cpp
    vulkan-renderer/wrapper/shader.cpp
    vulkan-renderer/wrapper/staging_buffer.cpp
    vulkan-renderer/wrapper/swapchain.cpp
    vulkan-renderer/wrapper/uniform_buffer.cpp
    vulkan-renderer/wrapper/window.cpp
    vulkan-renderer/wrapper/window_surface.cpp

    vulkan-renderer/world/cube.cpp
    vulkan-renderer/world/indentation.cpp)

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

add_library(inexor-vulkan-renderer ${INEXOR_SOURCE_FILES})

add_dependencies(inexor-vulkan-renderer inexor-shaders)

set_target_properties(
    inexor-vulkan-renderer PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(
    inexor-vulkan-renderer

    PUBLIC
    GLFW_INCLUDE_VULKAN
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    VMA_RECORDING_ENABLED=$<BOOL:${INEXOR_USE_VMA_RECORDING}>
)

if(MSVC)
    target_compile_options(inexor-vulkan-renderer PRIVATE "/MP")
endif()

target_include_directories(
    inexor-vulkan-renderer

    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party
)

target_link_libraries(
    inexor-vulkan-renderer

    PUBLIC
    ${CONAN_LIBS}
    Vulkan::Vulkan
)
