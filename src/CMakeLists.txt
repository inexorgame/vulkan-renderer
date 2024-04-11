set(INEXOR_SOURCE_FILES
    vulkan-renderer/application.cpp
    vulkan-renderer/bezier_curve.cpp
    vulkan-renderer/camera.cpp
    vulkan-renderer/exception.cpp
    vulkan-renderer/fps_counter.cpp
    vulkan-renderer/time_step.cpp

    vulkan-renderer/input/keyboard_mouse_data.cpp

    vulkan-renderer/io/byte_stream.cpp
    vulkan-renderer/io/nxoc_parser.cpp

    vulkan-renderer/renderers/imgui.cpp

    vulkan-renderer/render-graph/buffer.cpp
    vulkan-renderer/render-graph/graphics_pass.cpp
    vulkan-renderer/render-graph/graphics_pass_builder.cpp
    vulkan-renderer/render-graph/push_constant_range_resource.cpp
    vulkan-renderer/render-graph/render_graph.cpp
    vulkan-renderer/render-graph/texture.cpp

    vulkan-renderer/tools/cla_parser.cpp
    vulkan-renderer/tools/file.cpp

    vulkan-renderer/vk_tools/device_info.cpp
    vulkan-renderer/vk_tools/enumerate.cpp
    vulkan-renderer/vk_tools/gpu_info.cpp
    vulkan-renderer/vk_tools/representation.cpp

    vulkan-renderer/wrapper/device.cpp
    vulkan-renderer/wrapper/image.cpp
    vulkan-renderer/wrapper/instance.cpp
    vulkan-renderer/wrapper/make_info.cpp
    vulkan-renderer/wrapper/sampler.cpp
    vulkan-renderer/wrapper/shader.cpp
    vulkan-renderer/wrapper/swapchain.cpp
    vulkan-renderer/wrapper/window.cpp
    vulkan-renderer/wrapper/window_surface.cpp

    vulkan-renderer/wrapper/commands/command_buffer.cpp
    vulkan-renderer/wrapper/commands/command_pool.cpp

    vulkan-renderer/wrapper/descriptors/descriptor_pool.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_pool_allocator.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_set_updater.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_set_layout.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.cpp
    vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.cpp

    vulkan-renderer/wrapper/pipelines/pipeline.cpp
    vulkan-renderer/wrapper/pipelines/pipeline_builder.cpp
    vulkan-renderer/wrapper/pipelines/pipeline_layout.cpp

    vulkan-renderer/wrapper/synchronization/fence.cpp
    vulkan-renderer/wrapper/synchronization/semaphore.cpp

    vulkan-renderer/world/collision.cpp
    vulkan-renderer/world/collision_query.cpp
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
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(
    inexor-vulkan-renderer

    PUBLIC
    GLFW_INCLUDE_VULKAN
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    VK_NO_PROTOTYPES # Required by volk metaloader
)

# enable multi processor building if VS project
if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
    target_compile_options(inexor-vulkan-renderer PRIVATE "/MP")
endif()

# enable exceptions when using MSVC toolchain, makes Clang on windows possible
# enable correct reporting of the __cplusplus macro so correct code paths are selected
if(MSVC)
    target_compile_options(inexor-vulkan-renderer PRIVATE "-EHs")
    target_compile_options(inexor-vulkan-renderer PRIVATE "/Zc:__cplusplus")
endif()

# Extract the current git sha
find_package(Git REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --abbrev=7
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE INEXOR_GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Replace variables in header file with CMake values
configure_file(
    ${PROJECT_SOURCE_DIR}/include/inexor/vulkan-renderer/meta.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/inexor/vulkan-renderer/meta.hpp
)

target_include_directories(
    inexor-vulkan-renderer

    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    # Include configured header file which contains engine metadata
    ${CMAKE_CURRENT_BINARY_DIR}/include/
)

# Declare use of dependencies
FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(imgui)
add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(stb)
FetchContent_MakeAvailable(tinygltf)
FetchContent_MakeAvailable(toml)
FetchContent_MakeAvailable(vma)
FetchContent_MakeAvailable(volk)
FetchContent_MakeAvailable(Vulkan)

# Work around a VMA compile error from its use of snprintf
target_compile_definitions(VulkanMemoryAllocator PRIVATE VMA_STATS_STRING_ENABLED=0)

if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

target_link_libraries(inexor-vulkan-renderer
    PUBLIC
    fmt::fmt
    glfw
    glm::glm
    imgui
    spdlog::spdlog
    tinygltf
    toml11::toml11
    volk::volk
    Vulkan::Headers
    VulkanMemoryAllocator)
