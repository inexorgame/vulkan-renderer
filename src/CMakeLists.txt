add_library(
    inexor-vulkan-renderer

    vulkan-renderer/availability-checks/availability_checks.cpp
    vulkan-renderer/availability-checks/availability_checks_benchmarks.cpp
    vulkan-renderer/availability-checks/availability_checks_tests.cpp
    vulkan-renderer/camera/camera.cpp
    vulkan-renderer/command-buffer-recording/single_time_command_buffer.cpp
    vulkan-renderer/debug-marker/debug_marker_manager.cpp
    vulkan-renderer/descriptor-manager/descriptor_manager.cpp
    vulkan-renderer/error-handling/error_handling.cpp
    vulkan-renderer/fence-manager/fence_manager.cpp
    vulkan-renderer/fps-counter/fps_counter.cpp
    vulkan-renderer/gltf-model-manager/gltf_model_manager.cpp
    vulkan-renderer/gpu-info/gpu_info.cpp
    vulkan-renderer/gpu-queue-manager/gpu_queue_manager.cpp
    vulkan-renderer/mesh-buffer-manager/mesh_buffer_manager.cpp
    vulkan-renderer/semaphore-manager/semaphore_manager.cpp
    vulkan-renderer/settings-decision-maker/settings_decision_maker.cpp
    vulkan-renderer/shader-manager/shader_manager.cpp
    vulkan-renderer/texture/texture.cpp
    vulkan-renderer/texture-manager/texture_manager.cpp
    vulkan-renderer/thread-pool/thread_pool.cpp
    vulkan-renderer/time-step/time_step.cpp
    vulkan-renderer/tools/argument-parser/cla_parser.cpp
    vulkan-renderer/tools/file-loader/File.cpp
    vulkan-renderer/uniform-buffer-manager/uniform_buffer_manager.cpp
    vulkan-renderer/renderer.cpp
    inexor_application.cpp
)

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
    set_property(
        TARGET inexor-vulkan-renderer
        PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
endif()

target_include_directories(
    inexor-vulkan-renderer

    PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/third_party
)

target_link_libraries(
    inexor-vulkan-renderer

    PUBLIC
    ${CONAN_LIBS}
    Vulkan::Vulkan
)
