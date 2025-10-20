#pragma once

#include "inexor/vulkan-renderer/wrapper/commands/command_pool.hpp"

#include <array>
#include <functional>
#include <optional>
#include <span>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Instance;

using wrapper::commands::CommandBuffer;
using wrapper::commands::CommandPool;

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

    [[nodiscard]] auto device() const {
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

    [[nodiscard]] std::uint32_t transfer_queue_family_index() const {
        return m_transfer_queue_family_index;
    }

    /// Assign an internal Vulkan debug marker name to a Vulkan object.
    /// This internal name can be seen in external debuggers like RenderDoc.
    /// @note This method is only available in debug mode with ``VK_EXT_debug_marker`` device extension enabled.
    /// @param object The Vulkan object
    /// @param object_type The Vulkan debug report object type
    /// @param name The internal name of the Vulkan object
    void set_debug_marker_name(void *object, VkDebugReportObjectTypeEXT object_type, const std::string &name) const;

    /// Assigns a block of memory to a Vulkan resource.
    /// This memory block can be seen in external debuggers like RenderDoc.
    /// @note This method is only available in debug mode with ``VK_EXT_debug_marker`` device extension enabled.
    /// @param object The Vulkan object
    /// @param object_type The Vulkan debug report object type
    /// @param name The name of the memory block which will be connected to this object
    /// @param memory_size The size of the memory block in bytes
    /// @param memory_block The memory block to read from
    void set_memory_block_attachment(void *object, VkDebugReportObjectTypeEXT object_type, std::uint64_t name,
                                     std::size_t memory_size, const void *memory_block) const;

    /// Annotate a rendering region in Vulkan debug markers.
    /// The rendering region will be visible in external debuggers like RenderDoc.
    /// @param command_buffer The command buffer
    /// @param name The name of the rendering region
    /// @param color The rgba color of the rendering region
    void bind_debug_region(VkCommandBuffer command_buffer, const std::string &name, std::array<float, 4> color) const;

    /// Insert a debug markers into the current renderpass using vkCmdDebugMarkerInsertEXT.
    /// This debug markers can be seen in external debuggers like RenderDoc.
    /// @param command_buffer The command buffer which is associated to the debug marker
    /// @param name The name of the debug marker
    /// @param color An array of red, green, blue and alpha values for the debug region's color
    void insert_debug_marker(VkCommandBuffer command_buffer, const std::string &name, std::array<float, 4> color) const;

    /// End the debug region of the current renderpass using vkCmdDebugMarkerEndEXT.
    /// @param command_buffer The command buffer which is associated to the debug marker
    void end_debug_region(VkCommandBuffer command_buffer) const;

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
