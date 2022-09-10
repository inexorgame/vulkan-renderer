#pragma once

#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <cassert>
#include <functional>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// Call vkGetPhysicalDeviceMemoryProperties
/// @param physical_device The physical device
/// @return The memory properties of the physical device
[[nodiscard]] VkPhysicalDeviceMemoryProperties get_physical_device_memory_properties(VkPhysicalDevice physical_device);

/// Call vkGetPhysicalDeviceProperties
/// @param physical_device The physical device
/// @return The user-friendly name of the physical device
[[nodiscard]] std::string get_physical_device_name(VkPhysicalDevice physical_device);

/// Call vkGetPhysicalDeviceProperties
/// @param physical_device The physical device
/// @return The physical device type
[[nodiscard]] VkPhysicalDeviceType get_physical_device_type(VkPhysicalDevice physical_device);

/// Check if a certain device extension is available for a specific graphics card
/// @param physical_device The physical device
/// @param extension The name of the device extension
/// @return ``true`` if the requested device extension is available
[[nodiscard]] bool is_extension_supported(VkPhysicalDevice physical_device, const std::string &extension);

/// Call vkGetPhysicalDeviceSurfaceSupportKHR
/// @param physical_device The physical device
/// @param surface The window surface
/// @return ``true`` if presentation is supported
[[nodiscard]] bool is_presentation_supported(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

/// Call is_extension_supported with ``VK_KHR_SWAPCHAIN_EXTENSION_NAME``
/// @param physical_device The physical device
/// @return ``true`` if swapchain is supported
[[nodiscard]] bool is_swapchain_supported(VkPhysicalDevice physical_device);

/// Automatically select the best physical device (graphics card) out of all the available ones
/// The user can manually specify which graphics card will be used with command line argument ``--gpu <index>``
/// The method will check if the specified index is a valid array index
/// Please note that the graphics cards index starts with 0!
/// @param inst The Vulkan instance
/// @param surface The window surface
/// @param prefered_index The preferred graphics card index (starting with ``0``!)
/// @return The selected physical device (if any could be found), or std::nullopt otherwise
[[nodiscard]] std::optional<VkPhysicalDevice>
pick_graphics_card(VkInstance inst, VkSurfaceKHR surface, std::optional<std::uint32_t> prefered_index = std::nullopt);

/// Rate a graphics card by its features and properties
/// @param physical_device The physical device which will be rated
/// @param surface The window surface
/// @param type The physical device type
/// @param memory_props The memory properties of the physical device
/// @param swapchain_supported ``true`` if the physical device supports swapchains
/// @param presentation_supported ``true`` if the physical device supports presentation
/// @return The graphics card's score
/// @note If the score is smaller than ``0``, this means the physical device is unsuitable and can't be used!
[[nodiscard]] std::int32_t rate_physical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface,
                                                VkPhysicalDeviceType type,
                                                const VkPhysicalDeviceMemoryProperties &memory_props,
                                                bool swapchain_supported, bool presentation_supported);

/// RAII wrapper class for VkDevice, VkPhysicalDevice and VkQueues
/// @note There is no method ``is_layer_supported`` in this wrapper class because device layers are deprecated
class Device {
    VkDevice m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    VmaAllocator m_allocator{VK_NULL_HANDLE};
    std::string m_gpu_name;

    VkQueue m_graphics_queue{VK_NULL_HANDLE};
    VkQueue m_present_queue{VK_NULL_HANDLE};
    VkQueue m_transfer_queue{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

    std::uint32_t m_present_queue_family_index{0};
    std::uint32_t m_graphics_queue_family_index{0};
    std::uint32_t m_transfer_queue_family_index{0};

    /// According to NVidia, we should aim for one command pool per thread
    /// We will create one for graphics and one for transfer queue per thread
    /// https://developer.nvidia.com/blog/vulkan-dos-donts/
    mutable std::vector<std::unique_ptr<CommandPool>> m_cmd_pools;
    mutable std::mutex m_mutex;

    // The debug marker extension is not part of the core,
    // so function pointers need to be loaded manually.
    PFN_vkDebugMarkerSetObjectTagEXT m_vk_debug_marker_set_object_tag{nullptr};
    PFN_vkDebugMarkerSetObjectNameEXT m_vk_debug_marker_set_object_name{nullptr};
    PFN_vkCmdDebugMarkerBeginEXT m_vk_cmd_debug_marker_begin{nullptr};
    PFN_vkCmdDebugMarkerEndEXT m_vk_cmd_debug_marker_end{nullptr};
    PFN_vkCmdDebugMarkerInsertEXT m_vk_cmd_debug_marker_insert{nullptr};
    PFN_vkSetDebugUtilsObjectNameEXT m_vk_set_debug_utils_object_name{nullptr};

    const bool m_enable_vulkan_debug_markers{false};

    CommandPool &thread_graphics_pool() const;

public:
    /// @brief Default constructor.
    /// @param instance The instance wrapper from which the device will be created
    /// @param surface The surface which will be associated with the device
    /// @param enable_vulkan_debug_markers ``true`` if Vulkan debug markers should be enabled
    /// @param prefer_distinct_transfer_queue ``true`` if a distinct data transfer queue should be preferred
    /// @param preferred_physical_device_index The index of the preferred graphics card which should be used,
    /// starting from 0. If the graphics card index is invalid or if the graphics card is unsuitable for the
    /// application's purpose, another graphics card will be selected automatically. See the details of the device
    /// selection mechanism.
    /// TODO: Add overloaded constructors for VkPhysicalDeviceFeatures and requested device extensions
    Device(const wrapper::Instance &instance, VkSurfaceKHR surface, bool enable_vulkan_debug_markers,
           bool prefer_distinct_transfer_queue,
           std::optional<std::uint32_t> preferred_physical_device_index = std::nullopt);

    Device(const Device &) = delete;
    Device(Device &&) noexcept;

    ~Device();

    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) = delete;

    [[nodiscard]] VkDevice device() const {
        return m_device;
    }

    /// A wrapper method for beginning, ending and submitting command buffers
    /// This method calls the request method for the given command pool, begins the command buffer, executes the lambda,
    /// ends recording the command buffer, submits it and waits for it.
    /// @param name The internal debug name of the command buffer (must not be empty)
    /// @param cmd_lambda The command lambda to execute
    void execute(const std::string &name, const std::function<void(const CommandBuffer &cmd_buf)> &cmd_lambda) const;

    [[nodiscard]] VkPhysicalDevice physical_device() const {
        return m_physical_device;
    }

    [[nodiscard]] VmaAllocator allocator() const {
        return m_allocator;
    }

    [[nodiscard]] const std::string &gpu_name() const {
        return m_gpu_name;
    }

    [[nodiscard]] VkQueue graphics_queue() const {
        return m_graphics_queue;
    }

    [[nodiscard]] VkQueue present_queue() const {
        return m_present_queue;
    }

    /// @note Transfer queues are the fastest way to copy data across the PCIe bus. They are heavily underutilized
    /// even in modern games. Transfer queues can be used asynchronously to graphics queue.
    [[nodiscard]] VkQueue transfer_queue() const {
        return m_transfer_queue;
    }

    [[nodiscard]] std::uint32_t graphics_queue_family_index() const {
        return m_graphics_queue_family_index;
    }

    [[nodiscard]] std::uint32_t present_queue_family_index() const {
        return m_present_queue_family_index;
    }

    [[nodiscard]] std::uint32_t transfer_queue_family_index() const {
        return m_transfer_queue_family_index;
    }

    /// @brief Assign an internal Vulkan debug marker name to a Vulkan object.
    /// This internal name can be seen in external debuggers like RenderDoc.
    /// @note This method is only available in debug mode with ``VK_EXT_debug_marker`` device extension enabled.
    /// @param object The Vulkan object
    /// @param object_type The Vulkan debug report object type
    /// @param name The internal name of the Vulkan object
    void set_debug_marker_name(void *object, VkDebugReportObjectTypeEXT object_type, const std::string &name) const;

    /// @brief Assigns a block of memory to a Vulkan resource.
    /// This memory block can be seen in external debuggers like RenderDoc.
    /// @note This method is only available in debug mode with ``VK_EXT_debug_marker`` device extension enabled.
    /// @param object The Vulkan object
    /// @param object_type The Vulkan debug report object type
    /// @param name The name of the memory block which will be connected to this object
    /// @param memory_size The size of the memory block in bytes
    /// @param memory_block The memory block to read from
    void set_memory_block_attachment(void *object, VkDebugReportObjectTypeEXT object_type, std::uint64_t name,
                                     std::size_t memory_size, const void *memory_block) const;

    /// @brief Vulkan debug markers: Annotation of a rendering region.
    /// The rendering region will be visible in external debuggers like RenderDoc.
    /// @param command_buffer The associated command buffer
    /// @param name The name of the rendering region
    /// @param color The rgba color of the rendering region
    void bind_debug_region(VkCommandBuffer command_buffer, const std::string &name, std::array<float, 4> color) const;

    /// @brief Insert a debug markers into the current renderpass using vkCmdDebugMarkerInsertEXT.
    /// This debug markers can be seen in external debuggers like RenderDoc.
    /// @param command_buffer The command buffer which is associated to the debug marker
    /// @param name The name of the debug marker
    /// @param color An array of red, green, blue and alpha values for the debug region's color
    void insert_debug_marker(VkCommandBuffer command_buffer, const std::string &name, std::array<float, 4> color) const;

    /// @brief End the debug region of the current renderpass using vkCmdDebugMarkerEndEXT.
    /// @param command_buffer The command buffer which is associated to the debug marker
    void end_debug_region(VkCommandBuffer command_buffer) const;

    /// @brief Call vkCreateCommandPool
    /// @param command_pool_ci The command pool create info structure
    /// @param command_pool The command pool to create
    /// @param name The internal debug marker name which will be assigned to this command pool
    void create_command_pool(const VkCommandPoolCreateInfo &command_pool_ci, VkCommandPool *command_pool,
                             const std::string &name) const;

    /// @brief Call vkCreateDescriptorPool
    /// @param descriptor_pool_ci The descriptor pool create info structure
    /// @param descriptor_pool The descriptor pool to create
    /// @param name The internal debug marker name which will be assigned to this command pool
    void create_descriptor_pool(const VkDescriptorPoolCreateInfo &descriptor_pool_ci, VkDescriptorPool *descriptor_pool,
                                const std::string &name) const;

    /// @brief Call vkCreateDescriptorSetLayout
    /// @param descriptor_set_layout_ci The descriptor set layout create info structure
    /// @param descriptor_set_layout The descriptor set layout to create
    /// @param name The internal debug marker name which will be assigned to this descriptor set layout
    void create_descriptor_set_layout(const VkDescriptorSetLayoutCreateInfo &descriptor_set_layout_ci,
                                      VkDescriptorSetLayout *descriptor_set_layout, const std::string &name) const;

    /// @brief Call vkCreateFence
    /// @param fence_ci The fence create info structure
    /// @param fence The fence to create
    /// @param name The internal debug marker name which will be assigned to this fence
    void create_fence(const VkFenceCreateInfo &fence_ci, VkFence *fence, const std::string &name) const;

    /// @brief Call vkCreateFramebuffer
    /// @param framebuffer_ci The framebuffer create info structure
    /// @param framebuffer The Vulkan framebuffer to create
    /// @param name The internal debug marker name which will be assigned to this framebuffer
    void create_framebuffer(const VkFramebufferCreateInfo &framebuffer_ci, VkFramebuffer *framebuffer,
                            const std::string &name) const;

    /// @brief Call vkCreateGraphicsPipelines
    /// @param pipeline_ci The graphics pipeline create info structure
    /// @param pipeline The graphics pipeline to create
    /// @param name The internal debug marker name which will be assigned to this pipeline
    // TODO: Offer parameter for Vulkan pipeline caches!
    // TODO: Use std::span to offer a more general method (creating multiple pipelines with one call)
    // TODO: We might want to use std::span<std::pair<VkGraphicsPipelineCreateInfo, VkPipeline *>>
    void create_graphics_pipeline(const VkGraphicsPipelineCreateInfo &pipeline_ci, VkPipeline *pipeline,
                                  const std::string &name) const;

    /// @brief Call vkCreateImageView
    /// @param image_view_ci The image view create info structure
    /// @param image_view The image view to create
    /// @param name The internal debug marker name which will be assigned to this image view
    void create_image_view(const VkImageViewCreateInfo &image_view_ci, VkImageView *image_view,
                           const std::string &name) const;

    /// @brief Call vkCreatePipelineLayout
    /// @param pipeline_layout_ci The pipeline layout create info structure
    /// @param pipeline_layout The pipeline layout to create
    /// @param name The internal debug marker name which will be assigned to this pipeline layout
    void create_pipeline_layout(const VkPipelineLayoutCreateInfo &pipeline_layout_ci, VkPipelineLayout *pipeline_layout,
                                const std::string &name) const;

    /// @brief Call vkCreateRenderPass
    /// @param render_pass_ci The render pass create info structure
    /// @param render_pass The render pass to create
    /// @param name The internal debug marker name which will be assigned to this render pass
    void create_render_pass(const VkRenderPassCreateInfo &render_pass_ci, VkRenderPass *render_pass,
                            const std::string &name) const;

    /// @brief Call vkCreateSampler
    /// @param sampler_ci The sampler create info structure
    /// @param sampler The sampler to create
    /// @param name The internal debug marker name which will be assigned to this sampler
    void create_sampler(const VkSamplerCreateInfo &sampler_ci, VkSampler *sampler, const std::string &name) const;

    /// @brief Call vkCreateSemaphore
    /// @param semaphore_ci The semaphore create info structure
    /// @param semaphore The semaphore to create
    /// @param name The internal debug marker name which will be assigned to this semaphore
    void create_semaphore(const VkSemaphoreCreateInfo &semaphore_ci, VkSemaphore *semaphore,
                          const std::string &name) const;

    /// @brief Call vkCreateShaderModule
    /// @param shader_module_ci The shader module create info structure
    /// @param shader_module The shader module to create
    /// @param name The internal debug marker name which will be assigned to this shader module
    void create_shader_module(const VkShaderModuleCreateInfo &shader_module_ci, VkShaderModule *shader_module,
                              const std::string &name) const;

    /// @brief Call vkCreateSwapchainKHR
    /// @param swapchain_ci The swapchain_ci create info structure
    /// @param swapchain The swapchain to create
    /// @param name The internal debug marker name which will be assigned to this swapchain
    void create_swapchain(const VkSwapchainCreateInfoKHR &swapchain_ci, VkSwapchainKHR *swapchain,
                          const std::string &name) const;

    [[nodiscard]] const CommandBuffer &request_command_buffer(const std::string &name);
};

} // namespace inexor::vulkan_renderer::wrapper
