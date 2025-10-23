#pragma once

#include "inexor/vulkan-renderer/tools/representation.hpp"
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

    /// Set the debug name of a Vulkan object using debug utils extension (VK_EXT_debug_utils)
    /// @note We thought about overloading this method several times so the obj_type is set automatically depending on
    /// the type of the obj_handle you pass in, but it would make the code larger while being a little harder to
    /// understand what's really going on.
    /// @param obj_type The Vulkan object type
    /// @param obj_handle The Vulkan object handle (must not be nullptr!)
    /// @param name the internal debug name of the Vulkan object
    void set_debug_utils_object_name(VkObjectType obj_type, std::uint64_t obj_handle, const std::string &name) const;

    /// Get the thread_local command pool
    /// @note This method will create a command pool for the thread if it doesn't already exist
    CommandPool &thread_graphics_pool() const;

public:
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

    /// Request a command buffer from the thread_local command pool
    /// @param name The name which will be assigned to the command buffer
    /// @return A command buffer from the thread_local command pool
    [[nodiscard]] const CommandBuffer &request_command_buffer(const std::string &name);

    /// Check if a surface supports a certain image usage
    /// @param surface The window surface
    /// @param usage The requested image usage
    /// @return ``true`` if the format feature is supported
    [[nodiscard]] bool surface_supports_usage(VkSurfaceKHR surface, VkImageUsageFlagBits usage) const;

    /// Automatically detect the type of a Vulkan object and set the internal debug name to it
    /// @tparam VulkanObjectType The Vulkan object type. This template parameter will be automatically translated into
    /// the matching ``VkObjectType`` using ``vk_tools::get_vulkan_object_type(vk_object)``. This is the most advanced
    /// abstraction that we found and it's really easy to use set_debug_name now because it's not possible to make a
    /// mistake because you don't have to specify the VkObjectType manually when naming a Vulkan object.
    /// @param vk_object The Vulkan object to assign a name to
    /// @param name The internal debug name of the Vulkan object (must not be empty!)
    template <typename VulkanObjectType>
    void set_debug_name(const VulkanObjectType vk_object, const std::string &name) const {
        // The get_vulkan_object_type template allows us to convert the template parameter into a VK_OBJECT_TYPE
        // There is no other trivial way in C++ to do this as far as we know
        return set_debug_utils_object_name(tools::get_vulkan_object_type(vk_object),
                                           reinterpret_cast<std::uint64_t>(vk_object), name);
    }

    /// Call vkDeviceWaitIdle or vkQueueWaitIdle depending on whether ``queue`` is specified.
    /// @warning Avoid using those methods because they result in bad gpu performance!
    /// @param queue (``VK_NULL_HANDLE`` by default).
    /// @exception VulkanException ``vkDeviceWaitIdle`` call failed.
    /// @exception VulkanException ``vkQueueWaitIdle`` call failed.
    void wait_idle(VkQueue queue = VK_NULL_HANDLE) const;
};

} // namespace inexor::vulkan_renderer::wrapper
