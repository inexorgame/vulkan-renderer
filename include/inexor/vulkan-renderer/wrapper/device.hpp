#pragma once

#include "inexor/vulkan-renderer/vk_tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_pool.hpp"

#include <array>
#include <functional>
#include <optional>
#include <span>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declarations
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declarations
class CommandBuffer;
class CommandPool;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper {

/// The debug label colors for vkCmdBeginDebugUtilsLabelEXT
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

/// Convert a DebugLabelColor to an array of RGBA float values to pass to vkCmdBeginDebugUtilsLabelEXT
/// @param color The DebugLabelColor
/// @return An array of RGBA float values to be passed into vkCmdBeginDebugUtilsLabelEXT
[[nodiscard]] std::array<float, 4> get_debug_label_color(const DebugLabelColor color);

// Using declarations
using commands::CommandBuffer;
using commands::CommandPool;
using wrapper::Swapchain;

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
/// @note There is no method ``is_layer_supported`` in this wrapper class because device layers are deprecated
class Device {
    friend class CommandBuffer;
    friend class CommandPool;
    friend class Swapchain;

private:
    VkDevice m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    VmaAllocator m_allocator{VK_NULL_HANDLE};
    std::string m_gpu_name;
    VkPhysicalDeviceFeatures m_enabled_features{};
    VkPhysicalDeviceProperties m_properties{};
    VkSampleCountFlagBits m_max_usable_sample_count{VK_SAMPLE_COUNT_1_BIT};

    VkQueue m_compute_queue{VK_NULL_HANDLE};
    VkQueue m_graphics_queue{VK_NULL_HANDLE};
    VkQueue m_present_queue{VK_NULL_HANDLE};
    VkQueue m_transfer_queue{VK_NULL_HANDLE};

    std::uint32_t m_present_queue_family_index{0};
    std::uint32_t m_graphics_queue_family_index{0};
    std::uint32_t m_transfer_queue_family_index{0};
    std::uint32_t m_compute_queue_family_index{0};
    // TODO: Implement sparse binding queue if required

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

    /// Get the thread_local compute pool for transfer commands
    /// @note This method will create a command pool for the thread if it doesn't already exist
    const CommandPool &thread_local_command_pool(VkQueueFlagBits queue_type) const;

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
    static VkPhysicalDevice pick_best_physical_device(const Instance &inst,
                                                      VkSurfaceKHR surface,
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
    Device(const Instance &inst,
           VkSurfaceKHR surface,
           bool prefer_distinct_transfer_queue,
           VkPhysicalDevice physical_device,
           std::span<const char *> required_extensions,
           const VkPhysicalDeviceFeatures &required_features,
           const VkPhysicalDeviceFeatures &optional_features = {});

    Device(const Device &) = delete;
    // TODO: Implement me!
    Device(Device &&) noexcept;
    ~Device();

    Device &operator=(const Device &) = delete;
    // TODO: Implement me!
    Device &operator=(Device &&) noexcept;

    [[nodiscard]] VmaAllocator allocator() const {
        return m_allocator;
    }

    [[nodiscard]] VkDevice device() const {
        return m_device;
    }

    /// A wrapper method for beginning, ending and submitting command buffers. This method calls the request method for
    /// the given command pool, begins the command buffer, invokes the recording function, ends recording the command
    /// buffer, submits it on the specified queue and waits for it. Using this execute method is the preferred way of
    /// using command buffers in the engine. There is no need to request a command buffer manually, which is why this
    /// method in CommandPool is not public.
    ///
    /// @code{.cpp}
    /// m_device.execute("Upload Data", VK_QUEUE_TRANSFER_BIT, DebugLabelColor::RED,
    ///     [](const CommandBuffer &cmd_buf) { /*Do you vkCmd calls in here*/ }
    ///     /*Both could be a std::vector, an std::array, or std::span of VkSemaphore
    ///       It's also possible to submit one VkSemaphore as a std::span using {&wait_semaphore, 1}*/
    ///     wait_semaphores, signal_semaphores)
    /// @endcode
    ///
    /// @param name The internal debug name of the command buffer (must not be empty)
    /// @param queue_type The queue type to submit the command buffer to
    /// @param dbg_label_color The color of the debug label when calling ``begin_debug_label_region``
    /// @note Debug label colors are only visible in graphics debuggers such as RenderDoc
    /// @param cmd_buf_recording_func The command buffer recording function to invoke after starting recording
    /// @note It's technically allowed that the command buffer recording function is empty or a function which does not
    /// do any vkCmd command calls, but this makes no real sense because an empty command buffer will be submitted. It
    /// will not be checked if any commands have been recorded into the command buffer, although this could be
    /// implemented using CommandBuffer wrapper. However, this would be a case for validation layers though.
    /// @param wait_semaphores The semaphores to wait on before starting command buffer execution (empty by default)
    /// @param signal_semaphores The semaphores to signal once command buffer execution will finish (empty by default)
    void execute(const std::string &name,
                 VkQueueFlagBits queue_type,
                 DebugLabelColor dbg_label_color,
                 const std::function<void(const CommandBuffer &cmd_buf)> &cmd_buf_recording_func,
                 std::span<const VkSemaphore> wait_semaphores = {},
                 std::span<const VkSemaphore> signal_semaphores = {}) const;

    /// Find a queue family index that suits a specific criteria
    /// @param criteria_lambda The lambda to sort out unsuitable queue families
    /// @return The queue family index which was found (if any), ``std::nullopt`` otherwise
    std::optional<std::uint32_t> find_queue_family_index_if(
        const std::function<bool(std::uint32_t index, const VkQueueFamilyProperties &)> &criteria_lambda);

    /// Get the name of the physical device that was created
    [[nodiscard]] const std::string &gpu_name() const {
        return m_gpu_name;
    }

    /// Call vkGetPhysicalDeviceSurfaceSupportKHR
    /// @param surface The window surface
    /// @param queue_family_index The queue family index
    /// @exception VulkanException vkGetPhysicalDeviceSurfaceSupportKHR call failed
    /// @return ``true`` if presentation is supported
    [[nodiscard]] bool is_presentation_supported(VkSurfaceKHR surface, std::uint32_t queue_family_index) const;

    /// Returns the maximum sample count usable by the platform
    [[nodiscard]] VkSampleCountFlagBits max_usable_sample_count() const {
        return m_max_usable_sample_count;
    }

    /// Automatically detect the type of a Vulkan object and set the internal debug name to it
    /// @tparam VulkanObjectType The Vulkan object type. This template parameter will be automatically translated into
    /// the matching ``VkObjectType`` using ``vk_tools::get_vulkan_object_type(vk_object)``. This is the most advanced
    /// abstraction that we found and it's really easy to use set_debug_name now because it's not possible to make a
    /// mistake because you don't have to specify the VkObjectType manually when naming a Vulkan object.
    /// @param vk_object The Vulkan object to assign a name to
    /// @param name The internal debug name of the Vulkan object (must not be empty!)
    template <typename VulkanObjectType>
    void set_debug_name(const VulkanObjectType vk_object, const std::string &name) const {
        // The get_vulkan_object_type template allows us to turn the template parameter into a VK_OBJECT_TYPE
        // There is no other trivial way in C++ to do this as far as we know
        return set_debug_utils_object_name(vk_tools::get_vulkan_object_type(vk_object),
                                           reinterpret_cast<std::uint64_t>(vk_object), name);
    }

    /// Call vkDeviceWaitIdle or vkQueueWaitIdle if a VkQueue is specified as parameter
    /// @param A queue to wait on (``VK_NULL_HANDLE`` by default)
    /// @exception VulkanException vkDeviceWaitIdle call failed
    void wait_idle(VkQueue queue = VK_NULL_HANDLE) const;
};

} // namespace inexor::vulkan_renderer::wrapper
