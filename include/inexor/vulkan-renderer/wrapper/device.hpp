#pragma once

#include <vulkan/vulkan_core.h>

#include <array>
#include <cassert>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// @brief A RAII wrapper for VkDevice, VkPhysicalDevice and VkQueues.
class Device {
private:
    VkDevice device;
    VkPhysicalDevice graphics_card;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
    VkSurfaceKHR surface;

    std::uint32_t present_queue_family_index;
    std::uint32_t graphics_queue_family_index;
    std::uint32_t transfer_queue_family_index;

    // The debug marker extension is not part of the core,
    // so function pointers need to be loaded manually.
    PFN_vkDebugMarkerSetObjectTagEXT vk_debug_marker_set_object_tag;
    PFN_vkDebugMarkerSetObjectNameEXT vk_debug_marker_set_object_name;
    PFN_vkCmdDebugMarkerBeginEXT vk_cmd_debug_marker_begin;
    PFN_vkCmdDebugMarkerEndEXT vk_cmd_debug_marker_end;
    PFN_vkCmdDebugMarkerInsertEXT vk_cmd_debug_marker_insert;
    PFN_vkSetDebugUtilsObjectNameEXT vk_set_debug_utils_object_name;

public:
    /// Delete the copy constructor so shaders are move-only objects.
    Device(const Device &) = delete;
    Device(Device &&other) noexcept;

    /// Delete the copy assignment operator so shaders are move-only objects.
    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) noexcept = default;

    /// @brief Creates a graphics card interface.
    /// @param preferred_gpu_index [in] The index of the preferred physical device to use.
    Device(const VkInstance instance, const VkSurfaceKHR surface, bool enable_vulkan_debug_markers,
           bool prefer_distinct_transfer_queue,
           const std::optional<std::uint32_t> preferred_physical_device_index = std::nullopt);

    // TODO: Add overloaded constructors for VkPhysicalDeviceFeatures and requested device extensions in the future!

    ~Device();

    [[nodiscard]] const VkDevice get_device() const {
        assert(device);
        return device;
    }

    [[nodiscard]] const VkPhysicalDevice get_physical_device() const {
        assert(graphics_card);
        return graphics_card;
    }

    [[nodiscard]] const VkQueue get_graphics_queue() const {
        assert(graphics_queue);
        return graphics_queue;
    }

    [[nodiscard]] const VkQueue get_present_queue() const {
        assert(present_queue);
        return present_queue;
    }

    /// @note Transfer queues are the fastest way to copy data across the PCIe bus.
    /// They are heavily underutilized even in modern games.
    /// Transfer queues can be used asynchronously to graphics queuey.
    [[nodiscard]] const VkQueue get_transfer_queue() const {
        assert(transfer_queue);
        return transfer_queue;
    }

    [[nodiscard]] std::uint32_t get_graphics_queue_family_index() const {
        return graphics_queue_family_index;
    }

    [[nodiscard]] std::uint32_t get_present_queue_family_index() const {
        return present_queue_family_index;
    }

    [[nodiscard]] std::uint32_t get_transfer_queue_family_index() const {
        return transfer_queue_family_index;
    }

#ifndef NDEBUG

    /// @brief Vulkan debug marker: Sets the name of a Vulkan resource.
    /// The debug marker name will be visible in external debuggers like RenderDoc.
    /// @param object [in] A pointer to the Vulkan object.
    /// @param type [in] The type of the Vulkan object.
    /// @param name [in] The name of the debug marker which will be associated to the Vulkan object.
    void set_object_name(const std::uint64_t object, const VkDebugReportObjectTypeEXT type, const std::string& name);

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
