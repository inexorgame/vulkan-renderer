#pragma once

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <cassert>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// @class Device
/// @brief A RAII wrapper class for VkDevice, VkPhysicalDevice and VkQueues.
class Device {
    std::string m_gpu_name;
    VmaAllocator m_allocator;

    VkDevice m_device;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_graphics_card;

    VkQueue m_graphics_queue;
    VkQueue m_present_queue;
    VkQueue m_transfer_queue;

    std::uint32_t m_present_queue_family_index;
    std::uint32_t m_graphics_queue_family_index;
    std::uint32_t m_transfer_queue_family_index;

    // The debug marker extension is not part of the core,
    // so function pointers need to be loaded manually.
    PFN_vkDebugMarkerSetObjectTagEXT m_vk_debug_marker_set_object_tag;
    PFN_vkDebugMarkerSetObjectNameEXT m_vk_debug_marker_set_object_name;
    PFN_vkCmdDebugMarkerBeginEXT m_vk_cmd_debug_marker_begin;
    PFN_vkCmdDebugMarkerEndEXT m_vk_cmd_debug_marker_end;
    PFN_vkCmdDebugMarkerInsertEXT m_vk_cmd_debug_marker_insert;
    PFN_vkSetDebugUtilsObjectNameEXT m_vk_set_debug_utils_object_name;

    const bool m_enable_vulkan_debug_markers;

public:
    /// @brief Default constructor.
    /// @param instance [in] The Vulkan instance from which the device will be created.
    /// @param surface [in] The surface which will be associated with the device.
    /// @param enable_vulkan_debug_markers [in] True if Vulkan debug markers should be enabled, false otherwise.
    /// @param prefer_distinct_transfer_queue [in] True if a distinct data transfer queue (if available) should be
    /// enabled, false otherwise.
    /// @param preferred_physical_device_index [in] The index of the preferred graphics card which should be used,
    /// starting from 0. If the graphics card index is invalid or if the graphics card is unsuitable for the
    /// application's purpose, another graphics card will be selected automatically. See the details of the device
    /// selection mechanism!
    /// @todo Add overloaded constructors for VkPhysicalDeviceFeatures and requested device extensions in the future!
    Device(const VkInstance instance, const VkSurfaceKHR surface, bool enable_vulkan_debug_markers,
           bool prefer_distinct_transfer_queue,
           const std::optional<std::uint32_t> preferred_physical_device_index = std::nullopt);
    Device(const Device &) = delete;
    Device(Device &&) noexcept;
    ~Device();

    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) = default;

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

    /// @note Transfer queues are the fastest way to copy data across the PCIe bus. They are heavily underutilized even
    /// in modern games. Transfer queues can be used asynchronously to graphics queuey.
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

#ifndef NDEBUG

    /// For more information about Vulkan debugging tools check
    /// https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/
    /// Also check our RenderDoc's official website: https://renderdoc.org/

    /// @note Vulkan debug markers are only available in debug mode when VK_EXT_debug_marker device extension is used.
    /// @todo Add overloaded methods like "set_image_name" which accept a specific type instead of a pointer. This would
    /// also remove the need for a "type" argument.

    /// @brief Sets the internal name of a Vulkan resource using vkDebugMarkerSetObjectNameEXT. This internal name can
    /// be seen in external debuggers like RenderDoc.
    /// @param object [in] A pointer to the Vulkan object whose name will be set.
    /// @param type [in] The type of the Vulkan object.
    /// @param name [in] The internal name which will be assigned to the Vulkan object.
    void set_object_name(std::uint64_t object, const VkDebugReportObjectTypeEXT type, const std::string &name) const;

    /// @brief Assigns a memory block for debugging to a Vulkan resource using vkDebugMarkerSetObjectTagEXT. This memory
    /// block can later be seen in external debuggers like RenderDoc.
    /// @param object [in] A pointer to the Vulkan object to which a memory tag will be assigned.
    /// @param type [in] The type of the Vulkan object.
    /// @param name [in] The internal name of the object ag.
    /// @param tag_size [in] The size of the memory tag in bytes.
    /// @param tag [in] A pointer to the memory tag.
    void set_object_tag(const std::uint64_t object, const VkDebugReportObjectTypeEXT type, const std::uint64_t name,
                        const std::size_t tag_size, const void *tag) const;

    /// @brief Sets the color of the current rendering region using vkCmdDebugMarkerBeginEXT. This color can be seen in
    /// external debuggers like RenderDoc.
    /// @param command_buffer [in] The command buffer which is associated to the debug region.
    /// @param name [in] The name of the debug region.
    /// @param color [in] An array of red, green, blue and alpha values for the debug region's color.
    void bind_debug_region(const VkCommandBuffer command_buffer, const std::string &name,
                           const std::array<float, 4> color) const;

    /// @brief Inserts a debug markers into the current renderpass using vkCmdDebugMarkerInsertEXT. This debug markers
    /// can be seen in external debuggers like RenderDoc.
    /// @param command_buffer [in] The command buffer which is associated to the debug marker.
    /// @param name [in] The name of the debug marker.
    /// @param color [in] An array of red, green, blue and alpha values for the debug region's color.
    void insert_debug_marker(const VkCommandBuffer command_buffer, const std::string &name,
                             const std::array<float, 4> color) const;

    /// @brief Ends the debug region of the current renderpass using vkCmdDebugMarkerEndEXT.
    /// @param command_buffer [in] The command buffer which is associated to the debug marker.
    void end_debug_region(const VkCommandBuffer command_buffer) const;

#endif
};

} // namespace inexor::vulkan_renderer::wrapper
