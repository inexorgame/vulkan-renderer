#pragma once

#include "inexor/vulkan-renderer/wrapper/command_pool.hpp"

#include <array>
#include <functional>
#include <optional>
#include <span>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Instance;

/// A wrapper struct for physical device data
struct DeviceInfo {
    std::string name;
    VkPhysicalDevice physical_device{nullptr};
    VkPhysicalDeviceType type{VK_PHYSICAL_DEVICE_TYPE_OTHER};
    VkDeviceSize total_device_local{0};
    VkPhysicalDeviceFeatures features{};
    std::vector<VkExtensionProperties> extensions;
    bool presentation_supported{false};
    bool swapchain_supported{false};
};

/// A RAII wrapper class for VkDevice, VkPhysicalDevice and VkQueues
/// @note There is no method ``is_layer_supported`` in this wrapper class because device layers are deprecated.
class Device {
    VkDevice m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    VmaAllocator m_allocator{VK_NULL_HANDLE};
    std::string m_gpu_name;
    VkPhysicalDeviceFeatures m_enabled_features{};
    VkPhysicalDeviceProperties m_properties{};

    VkQueue m_graphics_queue{VK_NULL_HANDLE};
    VkQueue m_present_queue{VK_NULL_HANDLE};
    VkQueue m_transfer_queue{VK_NULL_HANDLE};

    std::uint32_t m_present_queue_family_index{0};
    std::uint32_t m_graphics_queue_family_index{0};
    std::uint32_t m_transfer_queue_family_index{0};

    /// According to NVidia, we should aim for one command pool per thread
    /// https://developer.nvidia.com/blog/vulkan-dos-donts/
    mutable std::vector<std::unique_ptr<CommandPool>> m_cmd_pools;
    mutable std::mutex m_mutex;

    // The debug marker extension is not part of the core, so function pointers need to be loaded manually
    PFN_vkDebugMarkerSetObjectTagEXT m_vk_debug_marker_set_object_tag{nullptr};
    PFN_vkDebugMarkerSetObjectNameEXT m_vk_debug_marker_set_object_name{nullptr};
    PFN_vkCmdDebugMarkerBeginEXT m_vk_cmd_debug_marker_begin{nullptr};
    PFN_vkCmdDebugMarkerEndEXT m_vk_cmd_debug_marker_end{nullptr};
    PFN_vkCmdDebugMarkerInsertEXT m_vk_cmd_debug_marker_insert{nullptr};
    PFN_vkSetDebugUtilsObjectNameEXT m_vk_set_debug_utils_object_name{nullptr};

    /// Get the thread_local command pool
    /// @note This method will create a command pool for the thread if it doesn't already exist
    CommandPool &thread_graphics_pool() const;

public:
    /// Pick the best physical device automatically
    /// @param physical_device_infos The data of the physical devices
    /// @param required_features The required device features
    /// @param required_extensions The required device extensions
    /// @exception std::runtime_error There are no physical devices are available at all
    /// @exception std::runtime_error No suitable physical device could be determined
    /// @return The chosen physical device which is most suitable
    static VkPhysicalDevice pick_best_physical_device(std::vector<DeviceInfo> &&physical_device_infos,
                                                      const VkPhysicalDeviceFeatures &required_features,
                                                      std::span<const char *> required_extensions);

    /// Pick the best physical device automatically
    /// @param inst The Vulkan instance
    /// @param surface The window surface
    /// @param required_features The required device features
    /// @param required_extensions The required device extensions
    /// @return The chosen physical device which is most suitable
    static VkPhysicalDevice pick_best_physical_device(const Instance &inst, VkSurfaceKHR surface,
                                                      const VkPhysicalDeviceFeatures &required_features,
                                                      std::span<const char *> required_extensions);

    /// Default constructor
    /// @param inst The Vulkan instance
    /// @param surface The window surface
    /// @param prefer_distinct_transfer_queue Specifies if a distinct transfer queue will be preferred
    /// @param physical_device The physical device
    /// @param required_extensions The required device extensions
    /// @param required_features The required device features which the physical device must all support
    /// @param optional_features The optional device features which do not necessarily have to be present
    /// @exception std::runtime_error The physical device is not suitable
    /// @exception std::runtime_error No graphics queue could be found
    /// @exception std::runtime_error No presentation queue could be found
    /// @exception VulkanException vkCreateDevice call failed
    /// @exception VulkanException vmaCreateAllocator call failed
    /// @note The creation of the physical device will not fail if one of the optional device features is not available
    Device(const Instance &inst, VkSurfaceKHR surface, bool prefer_distinct_transfer_queue,
           VkPhysicalDevice physical_device, std::span<const char *> required_extensions,
           const VkPhysicalDeviceFeatures &required_features, const VkPhysicalDeviceFeatures &optional_features = {});

    Device(const Device &) = delete;
    Device(Device &&) noexcept;

    ~Device();

    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) = delete;

    [[nodiscard]] VkDevice device() const {
        return m_device;
    }

    /// Call vkGetPhysicalDeviceSurfaceCapabilitiesKHR
    /// @param surface The window surface
    /// @exception VulkanException vkGetPhysicalDeviceSurfaceCapabilitiesKHR call failed
    /// @return The surface capabilities
    [[nodiscard]] VkSurfaceCapabilitiesKHR get_surface_capabilities(VkSurfaceKHR surface) const;

    /// Check if a format supports a feature for images created with ``VK_IMAGE_TILING_OPTIMAL``
    /// @param format The format
    /// @param feature The requested format feature
    /// @return ``true`` if the format feature is supported
    [[nodiscard]] bool format_supports_feature(VkFormat format, VkFormatFeatureFlagBits feature) const;

    /// Call vkGetPhysicalDeviceSurfaceSupportKHR
    /// @param surface The window surface
    /// @param queue_family_index The queue family index
    /// @exception VulkanException vkGetPhysicalDeviceSurfaceSupportKHR call failed
    /// @return ``true`` if presentation is supported
    [[nodiscard]] bool is_presentation_supported(VkSurfaceKHR surface, std::uint32_t queue_family_index) const;

    /// A wrapper method for beginning, ending and submitting command buffers. This method calls the request method for
    /// the given command pool, begins the command buffer, executes the lambda, ends recording the command buffer,
    /// submits it and waits for it.
    /// @param name The internal debug name of the command buffer (must not be empty)
    /// @param cmd_lambda The command lambda to execute
    void execute(const std::string &name, const std::function<void(const CommandBuffer &cmd_buf)> &cmd_lambda) const;

    /// Find a queue family index that suits a specific criteria
    /// @param criteria_lambda The lambda to sort out unsuitable queue families
    /// @return The queue family index which was found (if any), ``std::nullopt`` otherwise
    std::optional<std::uint32_t> find_queue_family_index_if(
        const std::function<bool(std::uint32_t index, const VkQueueFamilyProperties &)> &criteria_lambda);

    [[nodiscard]] VkPhysicalDevice physical_device() const {
        return m_physical_device;
    }

    [[nodiscard]] VmaAllocator allocator() const {
        return m_allocator;
    }

    /// @note Enabled features = required features + optional features which are supported
    [[nodiscard]] const VkPhysicalDeviceFeatures &enabled_device_features() const {
        return m_enabled_features;
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

    [[nodiscard]] VkQueue transfer_queue() const {
        return m_transfer_queue;
    }

    [[nodiscard]] std::uint32_t graphics_queue_family_index() const {
        return m_graphics_queue_family_index;
    }

    [[nodiscard]] std::uint32_t present_queue_family_index() const {
        return m_present_queue_family_index;
    }

    [[nodiscard]] VkPhysicalDeviceProperties physical_device_properties() const noexcept {
        return m_properties;
    }

    [[nodiscard]] std::uint32_t transfer_queue_family_index() const {
        return m_transfer_queue_family_index;
    }

    /// Call vkCreateCommandPool
    /// @param command_pool_ci The command pool create info structure
    /// @param command_pool The command pool to create
    /// @param name The internal debug marker name which will be assigned to this command pool
    void create_command_pool(const VkCommandPoolCreateInfo &command_pool_ci, VkCommandPool *command_pool,
                             const std::string &name) const;

    /// Call vkCreateDescriptorPool
    /// @param descriptor_pool_ci The descriptor pool create info structure
    /// @param descriptor_pool The descriptor pool to create
    /// @param name The internal debug marker name which will be assigned to this command pool
    void create_descriptor_pool(const VkDescriptorPoolCreateInfo &descriptor_pool_ci, VkDescriptorPool *descriptor_pool,
                                const std::string &name) const;

    /// Call vkCreateDescriptorSetLayout
    /// @param descriptor_set_layout_ci The descriptor set layout create info structure
    /// @param descriptor_set_layout The descriptor set layout to create
    /// @param name The internal debug marker name which will be assigned to this descriptor set layout
    void create_descriptor_set_layout(const VkDescriptorSetLayoutCreateInfo &descriptor_set_layout_ci,
                                      VkDescriptorSetLayout *descriptor_set_layout, const std::string &name) const;

    /// Call vkCreateFence
    /// @param fence_ci The fence create info structure
    /// @param fence The fence to create
    /// @param name The internal debug marker name which will be assigned to this fence
    void create_fence(const VkFenceCreateInfo &fence_ci, VkFence *fence, const std::string &name) const;

    /// Call vkCreateFramebuffer
    /// @param framebuffer_ci The framebuffer create info structure
    /// @param framebuffer The Vulkan framebuffer to create
    /// @param name The internal debug marker name which will be assigned to this framebuffer
    void create_framebuffer(const VkFramebufferCreateInfo &framebuffer_ci, VkFramebuffer *framebuffer,
                            const std::string &name) const;

    /// Call vkCreateGraphicsPipelines
    /// @param pipeline_ci The graphics pipeline create info structure
    /// @param pipeline The graphics pipeline to create
    /// @param name The internal debug marker name which will be assigned to this pipeline
    // TODO: Offer parameter for Vulkan pipeline caches!
    // TODO: Use std::span to offer a more general method (creating multiple pipelines with one call)
    // TODO: We might want to use std::span<std::pair<VkGraphicsPipelineCreateInfo, VkPipeline *>>
    void create_graphics_pipeline(const VkGraphicsPipelineCreateInfo &pipeline_ci, VkPipeline *pipeline,
                                  const std::string &name) const;

    /// Call vkCreateImageView
    /// @param image_view_ci The image view create info structure
    /// @param image_view The image view to create
    /// @param name The internal debug marker name which will be assigned to this image view
    void create_image_view(const VkImageViewCreateInfo &image_view_ci, VkImageView *image_view,
                           const std::string &name) const;

    /// Call vkCreatePipelineLayout
    /// @param pipeline_layout_ci The pipeline layout create info structure
    /// @param pipeline_layout The pipeline layout to create
    /// @param name The internal debug marker name which will be assigned to this pipeline layout
    void create_pipeline_layout(const VkPipelineLayoutCreateInfo &pipeline_layout_ci, VkPipelineLayout *pipeline_layout,
                                const std::string &name) const;

    /// Call vkCreateRenderPass
    /// @param render_pass_ci The render pass create info structure
    /// @param render_pass The render pass to create
    /// @param name The internal debug marker name which will be assigned to this render pass
    void create_render_pass(const VkRenderPassCreateInfo &render_pass_ci, VkRenderPass *render_pass,
                            const std::string &name) const;

    /// Call vkCreateSampler
    /// @param sampler_ci The sampler create info structure
    /// @param sampler The sampler to create
    /// @param name The internal debug marker name which will be assigned to this sampler
    void create_sampler(const VkSamplerCreateInfo &sampler_ci, VkSampler *sampler, const std::string &name) const;

    /// Call vkCreateSemaphore
    /// @param semaphore_ci The semaphore create info structure
    /// @param semaphore The semaphore to create
    /// @param name The internal debug marker name which will be assigned to this semaphore
    void create_semaphore(const VkSemaphoreCreateInfo &semaphore_ci, VkSemaphore *semaphore,
                          const std::string &name) const;

    /// Call vkCreateShaderModule
    /// @param shader_module_ci The shader module create info structure
    /// @param shader_module The shader module to create
    /// @param name The internal debug marker name which will be assigned to this shader module
    void create_shader_module(const VkShaderModuleCreateInfo &shader_module_ci, VkShaderModule *shader_module,
                              const std::string &name) const;

    /// Call vkCreateSwapchainKHR
    /// @param swapchain_ci The swapchain_ci create info structure
    /// @param swapchain The swapchain to create
    /// @param name The internal debug marker name which will be assigned to this swapchain
    void create_swapchain(const VkSwapchainCreateInfoKHR &swapchain_ci, VkSwapchainKHR *swapchain,
                          const std::string &name) const;

    /// Request a command buffer from the thread_local command pool
    /// @param name The name which will be assigned to the command buffer
    /// @return A command buffer from the thread_local command pool
    [[nodiscard]] const CommandBuffer &request_command_buffer(const std::string &name);

    /// Check if a surface supports a certain image usage
    /// @param surface The window surface
    /// @param usage The requested image usage
    /// @return ``true`` if the format feature is supported
    [[nodiscard]] bool surface_supports_usage(VkSurfaceKHR surface, VkImageUsageFlagBits usage) const;

    /// Call vkDeviceWaitIdle
    /// @exception VulkanException vkDeviceWaitIdle call failed
    void wait_idle() const;
};

} // namespace inexor::vulkan_renderer::wrapper
