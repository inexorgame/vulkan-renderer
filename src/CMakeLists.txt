add_library(
    inexor-vulkan-renderer

    vulkan-renderer/availability_checks.cpp
    vulkan-renderer/bezier_curve.cpp
    vulkan-renderer/single_time_command_buffer.cpp
    vulkan-renderer/debug_marker_manager.cpp
    vulkan-renderer/descriptor_manager.cpp
    vulkan-renderer/error_handling.cpp
    vulkan-renderer/fence_manager.cpp
    vulkan-renderer/fps_counter.cpp
    vulkan-renderer/gpu_info.cpp
    vulkan-renderer/gpu_queue_manager.cpp
    vulkan-renderer/application.cpp
    vulkan-renderer/mesh_buffer_manager.cpp
    vulkan-renderer/semaphore_manager.cpp
    vulkan-renderer/settings_decision_maker.cpp
    vulkan-renderer/shader_manager.cpp
    vulkan-renderer/texture.cpp
    vulkan-renderer/texture_manager.cpp
    vulkan-renderer/thread_pool.cpp
    vulkan-renderer/time_step.cpp
    vulkan-renderer/uniform_buffer_manager.cpp
    vulkan-renderer/renderer.cpp
    vulkan-renderer/gltf-model/manager.cpp

    vulkan-renderer/tools/cla_parser.cpp
    vulkan-renderer/tools/file.cpp

    vulkan-renderer/world/BitStream.cpp
    vulkan-renderer/world/Cube.cpp
)

add_dependencies(inexor-vulkan-renderer inexor-shaders)

set_target_properties(
    inexor-vulkan-renderer PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(
    inexor-vulkan-renderer

    PRIVATE
    GLFW_INCLUDE_VULKAN
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
