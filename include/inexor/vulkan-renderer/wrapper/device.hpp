#pragma once

#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <cassert>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// @brief A RAII wrapper class for VkDevice, VkPhysicalDevice and VkQueues.
/// @note There is no method ``is_layer_supported`` in this wrapper class because device layers are deprecated.
class Device {
    VkDevice m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_graphics_card{VK_NULL_HANDLE};
    VmaAllocator m_allocator{VK_NULL_HANDLE};
    std::string m_gpu_name;

    VkQueue m_graphics_queue{VK_NULL_HANDLE};
    VkQueue m_present_queue{VK_NULL_HANDLE};
    VkQueue m_transfer_queue{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

    std::uint32_t m_present_queue_family_index{0};
    std::uint32_t m_graphics_queue_family_index{0};
    std::uint32_t m_transfer_queue_family_index{0};

    // The debug marker extension is not part of the core,
    // so function pointers need to be loaded manually.
    PFN_vkDebugMarkerSetObjectTagEXT m_vk_debug_marker_set_object_tag{nullptr};
    PFN_vkDebugMarkerSetObjectNameEXT m_vk_debug_marker_set_object_name{nullptr};
    PFN_vkCmdDebugMarkerBeginEXT m_vk_cmd_debug_marker_begin{nullptr};
    PFN_vkCmdDebugMarkerEndEXT m_vk_cmd_debug_marker_end{nullptr};
    PFN_vkCmdDebugMarkerInsertEXT m_vk_cmd_debug_marker_insert{nullptr};
    PFN_vkSetDebugUtilsObjectNameEXT m_vk_set_debug_utils_object_name{nullptr};

    const bool m_enable_vulkan_debug_markers{false};

public:
    /// @brief Check if a certain device extension is available for a specific graphics card.
    /// @param graphics_card The graphics card.
    /// @param extension The name of the device extension.
    /// @return ``true`` if the requested device extension is available.
    [[nodiscard]] static bool is_extension_supported(VkPhysicalDevice graphics_card, const std::string &extension);

    /// @brief Check if a swapchain is available for a specific graphics card.
    /// @param graphics_card The graphics card.
    /// @return ``true`` if swapchain is supported.
    [[nodiscard]] static bool is_swapchain_supported(VkPhysicalDevice graphics_card);

    /// @brief Check if presentation is available for a specific combination of graphics card and surface.
    /// @param graphics_card The graphics card.
    /// @param surface The (window) surface.
    /// @return ``true`` if presentation is supported.
    [[nodiscard]] static bool is_presentation_supported(VkPhysicalDevice graphics_card, VkSurfaceKHR surface);

    /// @brief Default constructor.
    /// @param instance The instance wrapper from which the device will be created.
    /// @param surface The surface which will be associated with the device.
    /// @param enable_vulkan_debug_markers True if Vulkan debug markers should be enabled, false otherwise.
    /// @param prefer_distinct_transfer_queue True if a distinct data transfer queue (if available) should be
    /// enabled, false otherwise.
    /// @param preferred_physical_device_index The index of the preferred graphics card which should be used,
    /// starting from 0. If the graphics card index is invalid or if the graphics card is unsuitable for the
    /// application's purpose, another graphics card will be selected automatically. See the details of the device
    /// selection mechanism!
    /// @todo Add overloaded constructors for VkPhysicalDeviceFeatures and requested device extensions in the
    /// future!
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

    [[nodiscard]] VkPhysicalDevice physical_device() const {
        return m_graphics_card;
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
    /// @param object The Vulkan object.
    /// @param object_type The Vulkan debug report object type.
    /// @param name The internal name of the Vulkan object.
    void set_debug_marker_name(void *object, VkDebugReportObjectTypeEXT object_type, const std::string &name) const;

    /// @brief Assigns a block of memory to a Vulkan resource.
    /// This memory block can be seen in external debuggers like RenderDoc.
    /// @note This method is only available in debug mode with ``VK_EXT_debug_marker`` device extension enabled.
    /// @param object The Vulkan object.
    /// @param object_type The Vulkan debug report object type.
    /// @param name The name of the memory block which will be connected to this object.
    /// @param memory_size The size of the memory block in bytes.
    /// @param memory_address The memory address to read from.
    void set_memory_block_attachment(void *object, VkDebugReportObjectTypeEXT object_type, std::uint64_t name,
                                     std::size_t memory_size, const void *memory_block) const;

    /// @param color [in] The rgba color of the rendering region.
    /// @param name [in] The name of the rendering region.
    /// @param command_buffer [in] The associated command buffer.
    /// The rendering region will be visible in external debuggers like RenderDoc.
    /// @brief Vulkan debug markers: Annotation of a rendering region.
    void bind_debug_region(VkCommandBuffer command_buffer, const std::string &name, std::array<float, 4> color) const;

    /// @brief Insert a debug markers into the current renderpass using vkCmdDebugMarkerInsertEXT. This debug
    /// markers can be seen in external debuggers like RenderDoc.
    /// @param command_buffer The command buffer which is associated to the debug marker.
    /// @param name The name of the debug marker.
    /// @param color An array of red, green, blue and alpha values for the debug region's color.
    void insert_debug_marker(VkCommandBuffer command_buffer, const std::string &name, std::array<float, 4> color) const;

    /// @brief End the debug region of the current renderpass using vkCmdDebugMarkerEndEXT.
    /// @param command_buffer The command buffer which is associated to the debug marker.
    void end_debug_region(VkCommandBuffer command_buffer) const;
};

} // namespace inexor::vulkan_renderer::wrapper
