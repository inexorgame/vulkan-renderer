#pragma once

#include "inexor/vulkan-renderer/availability_checks.hpp"
#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/fps_counter.hpp"
#include "inexor/vulkan-renderer/frame_graph.hpp"
#include "inexor/vulkan-renderer/gpu_info.hpp"
#include "inexor/vulkan-renderer/msaa_target.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/time_step.hpp"

// Those components have been refactored to fulfill RAII idioms.
#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/fence.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/glfw_context.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/mesh_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"
#include "inexor/vulkan-renderer/wrapper/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/window.hpp"
#include "inexor/vulkan-renderer/wrapper/window_surface.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace inexor::vulkan_renderer {

class VulkanRenderer {
protected:
    // We try to avoid inheritance here and prefer a composition pattern.
    // TODO: VulkanSwapchainManager, VulkanPipelineManager, VulkanRenderPassManager?

    std::shared_ptr<VulkanGraphicsCardInfoViewer> gpu_info_manager = std::make_shared<VulkanGraphicsCardInfoViewer>();

    std::shared_ptr<AvailabilityChecksManager> availability_checks_manager =
        std::make_shared<AvailabilityChecksManager>();

    std::shared_ptr<VulkanSettingsDecisionMaker> settings_decision_maker =
        std::make_shared<VulkanSettingsDecisionMaker>();

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

    VkDebugReportCallbackEXT debug_report_callback {};

    bool debug_report_callback_initialised = false;

    TimeStep time_step;

    std::uint32_t window_width = 0;

    std::uint32_t window_height = 0;

    std::string window_title = "";

    FPSCounter fps_counter;

    // TODO: Refactor this!
    VkDescriptorBufferInfo uniform_buffer_info {};

    bool vsync_enabled = false;

    Camera game_camera;

    // RAII wrapper for glfw contexts.
    std::unique_ptr<wrapper::GLFWContext> glfw_context = nullptr;

    // RAII wrapper for glfw windows.
    std::unique_ptr<wrapper::Window> window = nullptr;

    // RAII wrapper for VkInstance.
    std::unique_ptr<wrapper::Instance> vkinstance = nullptr;

    // RAII wrapper for VkDevice, VkPhysicalDevice and VkQueues.
    std::unique_ptr<wrapper::Device> vkdevice = nullptr;

    // RAII wrapper for glfw compatible Vulkan surfaces.
    std::unique_ptr<wrapper::WindowSurface> surface = nullptr;

    // RAII wrapper for Swapchain.
    std::unique_ptr<wrapper::Swapchain> swapchain = nullptr;

    // RAII wrapper for command pools.
    std::unique_ptr<wrapper::CommandPool> command_pool = nullptr;

    std::unique_ptr<wrapper::Semaphore> image_available_semaphore;
    std::unique_ptr<wrapper::Semaphore> rendering_finished_semaphore;

    std::vector<wrapper::Shader> shaders;
    std::vector<wrapper::Texture> textures;
    std::vector<wrapper::UniformBuffer> uniform_buffers;
    std::vector<wrapper::MeshBuffer> mesh_buffers;
    std::vector<wrapper::Descriptor> descriptors;

    // NOTE: We use unique_ptr for easy frame graph recreation during swapchain invalidation
    std::unique_ptr<FrameGraph> m_frame_graph;

    std::vector<OctreeGpuVertex> m_octree_vertices;

    void setup_frame_graph();

    /// @brief Creates the semaphores neccesary for synchronisation.
    VkResult create_synchronisation_objects();

    VkResult create_descriptor_pool();

    VkResult create_descriptor_set_layouts();

    VkResult create_descriptor_writes();

    void recreate_swapchain();

    void render_frame();

public:
    ~VulkanRenderer();

    /// @brief Run Vulkan memory allocator's memory statistics
    void calculate_memory_budget();

    bool window_resized = false;

    /// Neccesary for taking into account the relative speed of the system's CPU.
    float time_passed = 0.0f;

    //
    TimeStep stopwatch;
};

} // namespace inexor::vulkan_renderer
