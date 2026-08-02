#pragma once

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_pool.hpp"

#include <vk_mem_alloc.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer::wrapper::pipelines {
// Forward declaration
class PipelineCache;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::wrapper::core {

// Forward declaration
class Instance;

// Using declarations
using commands::CommandBuffer;
using commands::CommandBufferBuilder;
using commands::CommandPool;
using tools::InexorException;
using tools::VulkanException;
using wrapper::pipelines::PipelineCache;

/// Debug label colors
enum class DebugLabelColor {
    RED,
    BLUE,
    GREEN,
    YELLOW,
    PURPLE,
    ORANGE,
    MAGENTA,
    CYAN,
    BROWN,
    PINK,
    LIME,
    TURQUOISE,
    BEIGE,
    MAROON,
    OLIVE,
    NAVY,
    TEAL,
};

struct QueueSemaphoreWait {
    VkSemaphore semaphore{VK_NULL_HANDLE};
    VkPipelineStageFlags2 stage_mask{VK_PIPELINE_STAGE_2_NONE};
};

/// Convert a DebugLabelColor to a rgba value
/// @param color The debug label color
/// @return The converted rgba values
[[nodiscard]] std::array<float, 4> get_debug_label_color(const DebugLabelColor color);

/// RAII wrapper class for `VkDevice`, `VkPhysicalDevice` and `VkQueue`.
/// @note There is no method ``is_layer_supported`` in this wrapper class because device layers are deprecated.
class Device {
    friend class CommandBuffer;
    friend class CommandPool;

private:
    VkDevice m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    std::unique_ptr<PipelineCache> m_pipeline_cache;
    VmaAllocator m_allocator{VK_NULL_HANDLE};
    std::string m_gpu_name;
    VkPhysicalDeviceFeatures m_enabled_features{};
    std::array<std::uint8_t, VK_UUID_SIZE> m_pipeline_cache_uuid{};

    VkQueue m_graphics_queue{VK_NULL_HANDLE};
    VkQueue m_transfer_queue{VK_NULL_HANDLE};
    VkQueue m_compute_queue{VK_NULL_HANDLE};
    VkQueue m_sparse_binding_queue{VK_NULL_HANDLE};

    std::optional<std::uint32_t> m_graphics_queue_family_index{0};
    std::optional<std::uint32_t> m_compute_queue_family_index{0};
    std::optional<std::uint32_t> m_transfer_queue_family_index{0};
    std::optional<std::uint32_t> m_sparse_binding_queue_family_index{0};
    std::optional<std::uint32_t> m_present_queue_family_index{0};

    /// According to NVidia, we should aim for one command pool per thread
    /// https://developer.nvidia.com/blog/vulkan-dos-donts/
    mutable std::vector<std::unique_ptr<CommandPool>> m_cmd_pools;
    mutable std::shared_mutex m_mutex;

    /// Get the thread_local command pool.
    /// @param queue_type The Vulkan queue type
    /// @note This method will create a command pool for the thread if it doesn't already exist.
    CommandPool &get_thread_command_pool(VkQueueFlagBits queue_type) const;

    // @TODO Implement get_thread_command_pool with "transfer if available, graphics otherwise" for copy operations.

public:
    /// Default constructor
    /// @param inst The Vulkan instance
    /// @param surface The window surface
    /// @param desired_gpu The physical device
    /// @param required_features The required device features which the physical device must all support
    /// @param required_extensions The required device extensions
    /// @exception std::runtime_error The physical device is not suitable
    /// @exception std::runtime_error No graphics queue could be found
    /// @exception std::runtime_error No presentation queue could be found
    /// @exception VulkanException vkCreateDevice call failed
    /// @exception VulkanException vmaCreateAllocator call failed
    /// @note The creation of the physical device will not fail if one of the optional device features is not available
    Device(const Instance &inst, VkSurfaceKHR surface, VkPhysicalDevice physical_device,
           const VkPhysicalDeviceFeatures &required_features, std::span<const char *> required_extensions);

    ~Device();

    [[nodiscard]] auto device() const {
        return m_device;
    }

    /// Call `vkGetPhysicalDeviceSurfaceCapabilitiesKHR`.
    /// @param surface The window surface.
    /// @exception VulkanException Calling `vkGetPhysicalDeviceSurfaceCapabilitiesKHR` failed
    /// @return The surface capabilities
    [[nodiscard]] VkSurfaceCapabilitiesKHR get_surface_capabilities(VkSurfaceKHR surface) const;

    /// Call `vkGetPhysicalDeviceSurfaceSupportKHR`.
    /// @note It was decided to place this function here into the device wrapper and not into the surface wrapper
    /// because the `VkPhysicalDevice` is required to call `vkGetPhysicalDeviceSurfaceSupportKHR`.
    /// @param surface The window surface.
    /// @param queue_family_index The queue family index.
    /// @exception VulkanException Calling `vkGetPhysicalDeviceSurfaceSupportKHR` failed.
    /// @return `true` if presentation is supported.
    [[nodiscard]] bool is_presentation_supported(VkSurfaceKHR surface, std::uint32_t queue_family_index) const;

    /// A wrapper method for beginning, ending and submitting command buffers. This method calls the request method for
    /// the given command pool, begins the command buffer, invokes the recording function, ends recording the command
    /// buffer, and submits it on the specified queue. The returned fence can be used by callers for synchronization.
    /// Using this execute method is the preferred way of
    /// using command buffers in the engine. There is no need to request a command buffer manually, which is why this
    /// method in CommandPool is not public.
    /// @param name The internal debug name of the command buffer (must not be empty)
    /// @param queue_type The queue type to submit the command buffer to
    /// @param dbg_label_color The color of the debug label when calling ``begin_debug_label_region``
    /// @note Debug label colors are only visible in graphics debuggers such as RenderDoc
    /// @param on_record The command buffer recording function to invoke after starting recording
    /// @note It's technically allowed that the command buffer recording function is empty or a function which does not
    /// do any vkCmd command calls, but this makes no real sense because an empty command buffer will be submitted. It
    /// will not be checked if any commands have been recorded into the command buffer, although this could be
    /// implemented using CommandBuffer wrapper. However, this would be a case for validation layers though.
    /// @param wait_semaphores The semaphores to wait on before starting command buffer execution (empty by default)
    /// @param signal_semaphores The semaphores to signal once command buffer execution will finish (empty by default)
    [[nodiscard]] VkFence execute(VkQueueFlagBits queue_type, DebugLabelColor dbg_label_color,
                                  const std::function<void(CommandBufferBuilder &cmd_buf)> &on_record,
                                  std::span<const VkSemaphore> wait_semaphores = {},
                                  std::span<const VkSemaphore> signal_semaphores = {},
                                  std::source_location source_location = std::source_location::current()) const;

    template <typename OnRecord>
    [[nodiscard]] VkFence execute(VkQueueFlagBits queue_type, DebugLabelColor dbg_label_color, OnRecord &&on_record,
                                  std::span<const VkSemaphore> wait_semaphores = {},
                                  std::span<const VkSemaphore> signal_semaphores = {},
                                  std::source_location source_location = std::source_location::current()) const {
        const auto &cmd_buf =
            get_thread_command_pool(queue_type).request_command_buffer(source_location.function_name());
        CommandBufferBuilder builder(cmd_buf);
        builder.begin_debug_label_region(source_location.function_name(), get_debug_label_color(dbg_label_color));
        std::invoke(on_record, builder);
        builder.end_debug_label_region();
        cmd_buf.end_command_buffer();
        cmd_buf.submit(queue_type, wait_semaphores, signal_semaphores);
        return cmd_buf.submission_fence();
    }

    [[nodiscard]] VkFence execute(VkQueueFlagBits queue_type, DebugLabelColor dbg_label_color,
                                  const std::function<void(CommandBufferBuilder &cmd_buf)> &on_record,
                                  std::span<const VkSemaphore> wait_semaphores,
                                  std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos,
                                  std::source_location source_location = std::source_location::current()) const;

    [[nodiscard]] VkFence execute(VkQueueFlagBits queue_type, DebugLabelColor dbg_label_color,
                                  const std::function<void(CommandBufferBuilder &cmd_buf)> &on_record,
                                  std::span<const QueueSemaphoreWait> wait_semaphores,
                                  std::span<const VkSemaphore> signal_semaphores = {},
                                  std::source_location source_location = std::source_location::current()) const;

    template <typename OnRecord>
    [[nodiscard]] VkFence execute(VkQueueFlagBits queue_type, DebugLabelColor dbg_label_color, OnRecord &&on_record,
                                  std::span<const QueueSemaphoreWait> wait_semaphores,
                                  std::span<const VkSemaphore> signal_semaphores = {},
                                  std::source_location source_location = std::source_location::current()) const {
        const auto &cmd_buf =
            get_thread_command_pool(queue_type).request_command_buffer(source_location.function_name());
        CommandBufferBuilder builder(cmd_buf);
        builder.begin_debug_label_region(source_location.function_name(), get_debug_label_color(dbg_label_color));
        std::invoke(on_record, builder);
        builder.end_debug_label_region();
        cmd_buf.end_command_buffer();

        cmd_buf.submit(queue_type, wait_semaphores, signal_semaphores);
        return cmd_buf.submission_fence();
    }

    [[nodiscard]] VkFence execute(VkQueueFlagBits queue_type, DebugLabelColor dbg_label_color,
                                  const std::function<void(CommandBufferBuilder &cmd_buf)> &on_record,
                                  std::span<const QueueSemaphoreWait> wait_semaphores,
                                  std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos,
                                  std::source_location source_location = std::source_location::current()) const;

    [[nodiscard]] VkPhysicalDevice physical_device() const {
        return m_physical_device;
    }

    [[nodiscard]] VmaAllocator allocator() const {
        return m_allocator;
    }

    [[nodiscard]] const VkPhysicalDeviceFeatures &enabled_features() const {
        return m_enabled_features;
    }

    [[nodiscard]] const std::string &gpu_name() const {
        return m_gpu_name;
    }

    [[nodiscard]] VkPipelineCache pipeline_cache() const;

    /// Get the pipeline cache UUID for the physical device
    /// @return A span view of the pipeline cache UUID bytes
    [[nodiscard]] std::span<const std::uint8_t, VK_UUID_SIZE> pipeline_cache_uuid() const {
        return m_pipeline_cache_uuid;
    }

    [[nodiscard]] bool has_any_compute_queue() const {
        return m_compute_queue != VK_NULL_HANDLE;
    }

    [[nodiscard]] bool has_any_transfer_queue() const {
        return m_transfer_queue != VK_NULL_HANDLE;
    }

    [[nodiscard]] bool transfer_queue_shares_graphics_family() const {
        return m_transfer_queue_family_index.has_value() && m_graphics_queue_family_index.has_value() &&
               m_transfer_queue_family_index.value() == m_graphics_queue_family_index.value();
    }

    [[nodiscard]] bool has_any_sparse_binding_queue() const {
        return m_transfer_queue != VK_NULL_HANDLE;
    }

    // TODO: Move to command buffer wrapper!
    [[nodiscard]] VkQueue compute_queue() const {
        return m_compute_queue;
    }

    // TODO: Move to command buffer wrapper!
    [[nodiscard]] VkQueue graphics_queue() const {
        return m_graphics_queue;
    }

    // TODO: Move to command buffer wrapper!
    [[nodiscard]] VkQueue transfer_queue() const {
        return m_transfer_queue;
    }

    [[nodiscard]] std::uint32_t graphics_queue_family_index() const {
        if (!m_graphics_queue_family_index.has_value()) {
            throw std::runtime_error("Error: Graphics queue family index is not available!");
        }
        return m_graphics_queue_family_index.value();
    }

    [[nodiscard]] std::uint32_t transfer_queue_family_index() const {
        if (!m_transfer_queue_family_index.has_value()) {
            throw std::runtime_error("Error: Transfer queue family index is not available!");
        }
        return m_transfer_queue_family_index.value();
    }

    /// Request a command buffer from the thread_local command pool.
    /// @param queue_type The Vulkan queue type which is required because a command pool is created with a queue family
    /// index associated with it.
    /// @param name The name which will be assigned to the command buffer.
    /// @return A command buffer from the thread_local command pool.
    [[nodiscard]] const CommandBuffer &request_command_buffer(VkQueueFlagBits queue_type, const std::string &name);

    /// Request a secondary command buffer from the thread_local command pool.
    /// @param queue_type The Vulkan queue type which is required because a command pool is created with a queue family
    /// index associated with it.
    /// @param name The name which will be assigned to the command buffer.
    /// @return A secondary command buffer from the thread_local command pool.
    [[nodiscard]] const CommandBuffer &request_secondary_command_buffer(VkQueueFlagBits queue_type,
                                                                        const std::string &name);

    /// Wait until all submitted command buffers in the current thread's pool for a queue type are complete.
    void wait_for_submissions(VkQueueFlagBits queue_type) const;

    /// Check if a surface supports a certain image usage.
    /// @param surface The window surface.
    /// @param usage The requested image usage.
    /// @return `true` if the format feature is supported.
    [[nodiscard]] bool surface_supports_usage(VkSurfaceKHR surface, VkImageUsageFlagBits usage) const;

    /// Automatically detect the type of a Vulkan object and set the internal debug name to it
    /// @tparam VulkanObjectType The Vulkan object type. This template parameter will be automatically translated into
    /// the matching `VkObjectType` using `vk_tools::get_vulkan_object_type(vk_object)`. This is the most advanced
    /// abstraction that we found and it's really easy to use `set_debug_name` now because it's not possible to make a
    /// mistake because you don't have to specify the VkObjectType manually when naming a Vulkan object.
    /// @param vk_object The Vulkan object to assign a name to.
    /// @param name The internal debug name of the Vulkan object (must not be empty!).
    template <typename VulkanObjectType>
    void set_debug_name(const VulkanObjectType &vk_object, const std::string &name) const {
        if (!vk_object) {
            throw InexorException("Error: Parameter 'vk_object' is invalid!");
        }

        const auto dbg_obj_name = tools::make_info<VkDebugUtilsObjectNameInfoEXT>({
            .objectType = tools::get_vk_object_type(vk_object),
            .objectHandle = reinterpret_cast<std::uint64_t>(vk_object),
            .pObjectName = name.c_str(),
        });

        if (const auto result = vkSetDebugUtilsObjectNameEXT(m_device, &dbg_obj_name); result != VK_SUCCESS) {
            throw VulkanException("Error: vkSetDebugUtilsObjectNameEXT failed!", result);
        }
    }

    /// Call vkUpdateDescriptorSets
    /// @param write_descriptor_sets The write descriptor sets
    void update_descriptor_sets(std::span<VkWriteDescriptorSet> write_descriptor_sets);

    /// Call `vkDeviceWaitIdle` or `vkQueueWaitIdle` depending on whether `queue` is specified.
    /// @warning Avoid using those methods because they result in bad gpu performance due to global stalls!
    /// @param queue (`VK_NULL_HANDLE` by default).
    /// @exception VulkanException `vkDeviceWaitIdle` call failed.
    /// @exception VulkanException `vkQueueWaitIdle` call failed.
    void wait_idle(VkQueue queue = VK_NULL_HANDLE) const;
};

} // namespace inexor::vulkan_renderer::wrapper::core
 
 
 
