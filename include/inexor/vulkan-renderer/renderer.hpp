#pragma once

#include "availability_checks.hpp"
#include "inexor/vulkan-renderer/camera.hpp"
#include "inexor/vulkan-renderer/debug_marker_manager.hpp"
#include "inexor/vulkan-renderer/descriptor.hpp"
#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/fence_manager.hpp"
#include "inexor/vulkan-renderer/fps_counter.hpp"
#include "inexor/vulkan-renderer/gpu_info.hpp"
#include "inexor/vulkan-renderer/gpu_queue_manager.hpp"
#include "inexor/vulkan-renderer/image_buffer.hpp"
#include "inexor/vulkan-renderer/mesh_buffer.hpp"
#include "inexor/vulkan-renderer/msaa_target.hpp"
#include "inexor/vulkan-renderer/octree_vertex.hpp"
#include "inexor/vulkan-renderer/semaphore_manager.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/texture.hpp"
#include "inexor/vulkan-renderer/time_step.hpp"
#include "inexor/vulkan-renderer/uniform_buffer.hpp"

// Those components have been refactored to fulfill RAII idioms.
#include "inexor/vulkan-renderer/shader.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <spdlog/spdlog.h>
#include <vma/vma_usage.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
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

    std::shared_ptr<VulkanQueueManager> gpu_queue_manager = std::make_shared<VulkanQueueManager>();

    std::shared_ptr<VulkanGraphicsCardInfoViewer> gpu_info_manager = std::make_shared<VulkanGraphicsCardInfoViewer>();

    std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager = std::make_shared<VulkanDebugMarkerManager>();

    std::shared_ptr<AvailabilityChecksManager> availability_checks_manager = std::make_shared<AvailabilityChecksManager>();

    std::shared_ptr<VulkanSettingsDecisionMaker> settings_decision_maker = std::make_shared<VulkanSettingsDecisionMaker>();

    VmaAllocator vma_allocator;

    VkInstance instance;

    VkDevice device;

    VkSurfaceKHR surface;

    VkPhysicalDevice selected_graphics_card;

    VkPresentModeKHR selected_present_mode;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    std::uint32_t number_of_images_in_swapchain = 0;

    VkSubmitInfo submit_info;

    VkPresentInfoKHR present_info = {};

    std::vector<VkImage> swapchain_images;

    std::vector<VkImageView> swapchain_image_views;

    VkPipelineLayout pipeline_layout = {};

    VkFormat selected_image_format = {};

    VkExtent2D swapchain_image_extent = {};

    VkColorSpaceKHR selected_color_space = {};

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

    VkRenderPass render_pass = VK_NULL_HANDLE;

    VkPipeline pipeline = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> frame_buffers;

    VkCommandPool command_pool = VK_NULL_HANDLE;

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

    GLFWwindow *window = nullptr;

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

    /// @brief Creates a Vulkan instance.
    /// @param application_name The name of the application
    /// @param engine_name The name of the engine.
    /// @param application_version The version of the application encoded as an unsigned 32 bit integer.
    /// @param engine_version The version of the engine encoded as an unsigned 32 bit integer.
    /// @param enable_validation_layers True if validation is enabled.
    VkResult create_vulkan_instance(const std::string &application_name, const std::string &engine_name, const std::uint32_t application_version,
                                    const std::uint32_t engine_version, bool enable_validation_instance_layers = true,
                                    bool enable_renderdoc_instance_layer = false);

    /// @brief Create a window surface.
    /// @param vulkan_instance The instance of Vulkan.
    /// @param window The GLFW window.
    /// @param vulkan_surface The Vulkan (window) surface.
    static VkResult create_window_surface(const VkInstance &vulkan_instance, GLFWwindow *window, VkSurfaceKHR &vulkan_surface);

    /// @brief Create a physical device handle.
    /// @param graphics_card The regarded graphics card.
    VkResult create_physical_device(const VkPhysicalDevice &graphics_card, const bool enable_debug_markers = true);

    /// @brief Creates an instance of VulkanDebugMarkerManager
    VkResult initialise_debug_marker_manager(const bool enable_debug_markers = true);

    /// @brief Initialises glTF 2.0 model manager.
    VkResult initialise_glTF2_model_manager();

    VkResult update_cameras();

    /// @brief Initialise allocator of Vulkan Memory Allocator library.
    VkResult create_vma_allocator();

    /// @brief Create depth image.
    VkResult create_depth_buffer();

    /// @brief Creates the command buffers.
    VkResult create_command_buffers();

    /// @brief Records the command buffers.
    VkResult record_command_buffers();

    /// @brief Creates the semaphores neccesary for synchronisation.
    VkResult create_synchronisation_objects();

    /// @brief Creates the swapchain.
    VkResult create_swapchain();

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
