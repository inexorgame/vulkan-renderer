#pragma once

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <cassert>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// @brief A RAII wrapper for VkDevice, VkPhysicalDevice and VkQueues.
class Device {
private:
    VkDevice m_device;
    VkPhysicalDevice m_graphics_card;
    VmaAllocator m_allocator;

    // TODO(): Create a queue wrapper
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;
    VkQueue m_transfer_queue;
    VkSurfaceKHR m_surface;

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

    bool m_enable_vulkan_debug_markers;

public:
    /// @brief Creates a graphics card interface.
    /// @param preferred_gpu_index [in] The index of the preferred physical device to use.
    Device(const VkInstance instance, const VkSurfaceKHR surface, bool enable_vulkan_debug_markers,
           bool prefer_distinct_transfer_queue,
           const std::optional<std::uint32_t> preferred_physical_device_index = std::nullopt);
    Device(const Device &) = delete;
    Device(Device &&) noexcept;
    ~Device();

    // TODO: Add overloaded constructors for VkPhysicalDeviceFeatures and requested device extensions in the future!

    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) = default;

    [[nodiscard]] const VkDevice device() const {
        assert(m_device);
        return m_device;
    }

    [[nodiscard]] const VkPhysicalDevice physical_device() const {
        assert(m_graphics_card);
        return m_graphics_card;
    }

    [[nodiscard]] VmaAllocator allocator() const {
        return m_allocator;
    }

    [[nodiscard]] const VkQueue graphics_queue() const {
        assert(m_graphics_queue);
        return m_graphics_queue;
    }

    [[nodiscard]] const VkQueue present_queue() const {
        assert(m_present_queue);
        return m_present_queue;
    }

    /// @note Transfer queues are the fastest way to copy data across the PCIe bus.
    /// They are heavily underutilized even in modern games.
    /// Transfer queues can be used asynchronously to graphics queuey.
    [[nodiscard]] const VkQueue transfer_queue() const {
        assert(m_transfer_queue);
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

    /// @brief Vulkan debug marker: Sets the name of a Vulkan resource.
    /// The debug marker name will be visible in external debuggers like RenderDoc.
    /// @param object [in] A pointer to the Vulkan object.
    /// @param type [in] The type of the Vulkan object.
    /// @param name [in] The name of the debug marker which will be associated to the Vulkan object.
    void set_object_name(const std::uint64_t object, const VkDebugReportObjectTypeEXT type, const std::string &name);

    /// @brief Vulkan debug marker: Links a memory dump block to a Vulkan resource.
    /// The object will be visible in external debuggers like RenderDoc.
    /// @param object [in] A pointer to the Vulkan object.
    /// @param type [in] The type of the Vulkan object.
    /// @param name [in] The name of the debug marker which will be associated to the Vulkan object.
    /// @param tag_size [in] The size of the memory dump.
    /// @param tag [in] A pointer to the memory dump.
    void set_object_tag(const std::uint64_t object, const VkDebugReportObjectTypeEXT type, const std::uint64_t name,
                        const std::size_t tag_size, const void *tag);

    /// @brief Vulkan debug markers: Annotation of a rendering region.
    /// The rendering region will be visible in external debuggers like RenderDoc.
    /// @param command_buffer [in] The associated command buffer.
    /// @param name [in] The name of the rendering region.
    /// @param color [in] The rgba color of the rendering region.
    void bind_debug_region(const VkCommandBuffer command_buffer, const std::string &name,
                           const std::array<float, 4> color);

    /// @brief Vulkan debug markers: Inserts a debug marker into a renderpass.
    /// The debug marker will be visible in external debuggers like RenderDoc.
    /// @param command_buffer [in] The associated command buffer.
    /// @param name [in] The name of the rendering region.
    /// @param color [in] The rgba color of the rendering region.
    void insert_debug_marker(const VkCommandBuffer command_buffer, const std::string &name,
                             const std::array<float, 4> color);

    /// @brief Vulkan debug markers: Annotation of a rendering region.
    /// The rendering region will be visible in external debuggers like RenderDoc.
    /// @param command_buffer [in] The associated command buffer.
    /// @param name [in] The name of the rendering region.
    /// @param color [in] The rgba color of the rendering region.
    void end_debug_region(const VkCommandBuffer command_buffer);

#endif
};

} // namespace inexor::vulkan_renderer::wrapper
