﻿#pragma once

#include "inexor/vulkan-renderer/availability_checks.hpp"
#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/fps_counter.hpp"
#include "inexor/vulkan-renderer/frame_graph.hpp"
#include "inexor/vulkan-renderer/gpu_info.hpp"
#include "inexor/vulkan-renderer/msaa_target.hpp"
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
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/mesh_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"
#include "inexor/vulkan-renderer/wrapper/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/vma.hpp"
#include "inexor/vulkan-renderer/wrapper/window.hpp"
#include "inexor/vulkan-renderer/wrapper/window_surface.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace inexor::vulkan_renderer {

// The maximum number of images to process simultaneously.
// TODO: Refactoring! That is triple buffering essentially!
constexpr unsigned int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanRenderer {
private:
    FrameGraph m_frame_graph;

protected:
    // We try to avoid inheritance here and prefer a composition pattern.
    // TODO: VulkanSwapchainManager, VulkanPipelineManager, VulkanRenderPassManager?

    std::shared_ptr<VulkanGraphicsCardInfoViewer> gpu_info_manager = std::make_shared<VulkanGraphicsCardInfoViewer>();

    std::shared_ptr<AvailabilityChecksManager> availability_checks_manager =
        std::make_shared<AvailabilityChecksManager>();

    std::shared_ptr<VulkanSettingsDecisionMaker> settings_decision_maker =
        std::make_shared<VulkanSettingsDecisionMaker>();

    VkPresentModeKHR selected_present_mode;

    VkSubmitInfo submit_info;

    VkPresentInfoKHR present_info = {};

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

    std::vector<wrapper::CommandBuffer> command_buffers;

    std::vector<wrapper::Semaphore> image_available_semaphores;

    std::vector<wrapper::Semaphore> rendering_finished_semaphores;

    std::vector<wrapper::Fence> in_flight_fences;

    VkDebugReportCallbackEXT debug_report_callback = {};

    bool debug_report_callback_initialised = false;

    std::uint32_t vma_dump_index = 0;

    TimeStep time_step;

    std::uint32_t window_width = 0;

    std::uint32_t window_height = 0;

    std::string window_title = "";

    FPSCounter fps_counter;

    // TODO: Refactor this!
    VkDescriptorBufferInfo uniform_buffer_info = {};
    VkDescriptorImageInfo image_info = {};
    VkPipelineCache pipeline_cache;

    // TODO: Read from TOML configuration file and pass value to core engine.
    bool multisampling_enabled = true;

    VkSampleCountFlagBits multisampling_sample_count = VK_SAMPLE_COUNT_4_BIT;

    MSAATarget msaa_target_buffer;

    bool vsync_enabled = false;

    Camera game_camera;

    std::vector<wrapper::Shader> shaders;
    std::vector<wrapper::Texture> textures;
    std::vector<wrapper::UniformBuffer> uniform_buffers;
    std::vector<wrapper::MeshBuffer> mesh_buffers;
    std::vector<wrapper::Descriptor> descriptors;

    // TODO(Hanni): Remove this with RAII refactoring of descriptors!
    VkDescriptorImageInfo descriptor_image_info = {};

    // RAII wrapper for VkInstance.
    std::unique_ptr<wrapper::Instance> vkinstance = nullptr;

    // RAII wrapper for VkDevice, VkPhysicalDevice and VkQueues.
    std::unique_ptr<wrapper::Device> vkdevice = nullptr;

    // RAII wrapper for Swapchain.
    std::unique_ptr<wrapper::Swapchain> swapchain = nullptr;

    // RAII wrapper for Vulkan Memory Allocator.
    std::unique_ptr<wrapper::VulkanMemoryAllocator> vma = nullptr;

    // RAII wrapper for glfw windows.
    std::unique_ptr<wrapper::Window> window = nullptr;

    // RAII wrapper for glfw compatible Vulkan surfaces.
    std::unique_ptr<wrapper::WindowSurface> surface = nullptr;

    /// RAII wrapper for glfw contexts.
    std::unique_ptr<wrapper::GLFWContext> glfw_context = nullptr;

    /// RAII wrapper for command pools.
    std::unique_ptr<wrapper::CommandPool> command_pool = nullptr;

    std::unique_ptr<wrapper::PipelineLayout> pipeline_layout;

    std::unique_ptr<wrapper::Framebuffer> framebuffer;

    /// @brief Create a physical device handle.
    /// @param graphics_card The regarded graphics card.
    VkResult create_physical_device(const VkPhysicalDevice &graphics_card, const bool enable_debug_markers = true);

    VkResult update_cameras();

    /// @brief Creates the semaphores neccesary for synchronisation.
    VkResult create_synchronisation_objects();

    /// @brief Cleans the swapchain.
    VkResult cleanup_swapchain();

    /// @brief Creates the uniform buffers.
    VkResult create_uniform_buffers();

    VkResult create_descriptor_pool();

    VkResult create_descriptor_set_layouts();

    VkResult create_descriptor_writes();

    /// @brief Creates the descriptor set.
    VkResult create_descriptor_sets();

    /// @brief Recreates the swapchain.
    VkResult recreate_swapchain();

    /// @brief Creates the command pool.
    VkResult create_command_pool();

    void create_frame_graph();

    /// @brief Destroys all Vulkan objects.
    VkResult shutdown_vulkan();

public:
    VulkanRenderer() = default;
    ~VulkanRenderer() = default;

    /// @brief Run Vulkan memory allocator's memory statistics.
    VkResult calculate_memory_budget();

    // Although many drivers and platforms trigger VK_ERROR_OUT_OF_DATE_KHR automatically after a window resize,
    // it is not guaranteed to happen. That’s why we’ll add some extra code to also handle resizes explicitly.
    bool frame_buffer_resized = false;

    /// Neccesary for taking into account the relative speed of the system's CPU.
    float time_passed = 0.0f;

    //
    TimeStep stopwatch;
};

} // namespace inexor::vulkan_renderer
