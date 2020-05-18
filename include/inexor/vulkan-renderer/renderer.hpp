#pragma once

#include "inexor/vulkan-renderer/availability_checks.hpp"
#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/descriptor.hpp"
#include "inexor/vulkan-renderer/fence_manager.hpp"
#include "inexor/vulkan-renderer/fps_counter.hpp"
#include "inexor/vulkan-renderer/gpu_info.hpp"
#include "inexor/vulkan-renderer/image_buffer.hpp"
#include "inexor/vulkan-renderer/mesh_buffer.hpp"
#include "inexor/vulkan-renderer/msaa_target.hpp"
#include "inexor/vulkan-renderer/semaphore_manager.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/shader.hpp"
#include "inexor/vulkan-renderer/texture.hpp"
#include "inexor/vulkan-renderer/time_step.hpp"
#include "inexor/vulkan-renderer/uniform_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/glfw_context.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"
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
constexpr unsigned int MAX_FRAMES_IN_FLIGHT = 3;

class VulkanRenderer {
protected:
    // We try to avoid inheritance here and prefer a composition pattern.
    // TODO: VulkanSwapchainManager, VulkanPipelineManager, VulkanRenderPassManager?

    std::shared_ptr<VulkanFenceManager> fence_manager = std::make_shared<VulkanFenceManager>();

    std::shared_ptr<VulkanSemaphoreManager> semaphore_manager = std::make_shared<VulkanSemaphoreManager>();

    std::shared_ptr<VulkanGraphicsCardInfoViewer> gpu_info_manager = std::make_shared<VulkanGraphicsCardInfoViewer>();

    std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager = std::make_shared<VulkanDebugMarkerManager>();

    std::shared_ptr<AvailabilityChecksManager> availability_checks_manager = std::make_shared<AvailabilityChecksManager>();

    std::shared_ptr<VulkanSettingsDecisionMaker> settings_decision_maker = std::make_shared<VulkanSettingsDecisionMaker>();

    VkPresentModeKHR selected_present_mode;

    VkSubmitInfo submit_info;

    VkPresentInfoKHR present_info = {};

    VkPipelineLayout pipeline_layout = {};

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

    VkRenderPass render_pass = VK_NULL_HANDLE;

    VkPipeline pipeline = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> frame_buffers;

    std::vector<VkCommandBuffer> command_buffers;

    std::vector<std::shared_ptr<VkSemaphore>> image_available_semaphores;

    std::vector<std::shared_ptr<VkSemaphore>> rendering_finished_semaphores;

    std::vector<std::shared_ptr<VkFence>> in_flight_fences;

    std::vector<std::shared_ptr<VkFence>> images_in_flight;

    VkDebugReportCallbackEXT debug_report_callback = {};

    bool debug_report_callback_initialised = false;

    ImageBuffer depth_buffer;

    ImageBuffer depth_stencil;

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

    std::vector<Shader> shaders;
    std::vector<Texture> textures;
    std::vector<UniformBuffer> uniform_buffers;
    std::vector<MeshBuffer> mesh_buffers;
    std::vector<Descriptor> descriptors;

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

    /// @brief Create a physical device handle.
    /// @param graphics_card The regarded graphics card.
    VkResult create_physical_device(const VkPhysicalDevice &graphics_card, const bool enable_debug_markers = true);

    /// @brief Creates an instance of VulkanDebugMarkerManager.
    VkResult initialise_debug_marker_manager(const bool enable_debug_markers = true);

    VkResult update_cameras();

    /// @brief Create depth image.
    VkResult create_depth_buffer();

    /// @brief Creates the command buffers.
    VkResult create_command_buffers();

    /// @brief Records the command buffers.
    VkResult record_command_buffers();

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

    /// @brief Creates the frame buffers.
    VkResult create_frame_buffers();

    /// @brief Creates the rendering pipeline.
    VkResult create_pipeline();

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
