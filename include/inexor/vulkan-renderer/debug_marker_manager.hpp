#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer {

// Predefined color markers.
// These colors will be visible in RenderDoc.
constexpr glm::vec4 DEBUG_MARKER_BLUE = glm::vec4(0.0f, 148 / 255, 1.0f, 1.0f);
constexpr glm::vec4 DEBUG_MARKER_RED = glm::vec4(1.0f, 0.0f, 21 / 255, 1.0f);
constexpr glm::vec4 DEBUG_MARKER_YELLOW = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
constexpr glm::vec4 DEBUG_MARKER_PURPLE = glm::vec4(1.0f, 0.0f, 180 / 255, 1.0f);
constexpr glm::vec4 DEBUG_MARKER_GREEN = glm::vec4(40 / 255, 210 / 255, 0.0f, 1.0f);
constexpr glm::vec4 DEBUG_MARKER_ORANGE = glm::vec4(1.0f, 100 / 255, 0.0f, 1.0f);

/// @brief A manager class for Vulkan debug markers.
/// Debug markers are very useful because they allow single steps of the
/// rendering process to be tracked by external debugging tools like RenderDoc.
class VulkanDebugMarkerManager {
private:
    bool active = false;

    bool extension_present = false;

    PFN_vkDebugMarkerSetObjectTagEXT marker_set_object_tag = VK_NULL_HANDLE;

    PFN_vkDebugMarkerSetObjectNameEXT marker_set_object_name = VK_NULL_HANDLE;

    PFN_vkCmdDebugMarkerBeginEXT cmd_marker_begin = VK_NULL_HANDLE;

    PFN_vkCmdDebugMarkerInsertEXT cmd_marker_insert = VK_NULL_HANDLE;

    PFN_vkCmdDebugMarkerEndEXT cmd_marker_end = VK_NULL_HANDLE;

public:
    VulkanDebugMarkerManager() = default;

    ~VulkanDebugMarkerManager() = default;

    /// @brief Initialises Vulkan debug marker manager.
    /// @param device [in] The Vulkan device.
    /// @param graphics_card [in] The graphics card.
    /// @brief enable_debug_markers [in] True if debug markers are enabled, false otherwise.
    void init(const VkDevice &device, const VkPhysicalDevice &graphics_card, bool enable_debug_markers = true);

    /// @brief Sets the debug name of an object.
    /// All Objects in Vulkan are represented by their 64-bit handles which are passed into this function along with the object type
    void set_object_name(const VkDevice &device, const std::uint64_t &object, const VkDebugReportObjectTypeEXT &object_type, const char *name);

    /// @brief Sets the tag for an object.
    /// @note We can link a memory block of arbitrary size to an object.
    void set_object_tag(const VkDevice &device, const std::uint64_t &object, const VkDebugReportObjectTypeEXT &object_type, const std::uint64_t &name,
                        const std::size_t &tag_size, const void *tag);

    /// Starts a new debug marker region.
    void bind_region(const VkCommandBuffer &cmdbuffer, const std::string &debug_marker_name, const glm::vec4 &debug_marker_color);

    /// @brief Inserts a new debug marker into the command buffer.
    void insert(const VkCommandBuffer &command_buffer, const std::string &debug_marker_name, const glm::vec4 &debug_marker_color);

    /// Ends the current debug marker region.
    void end_region(const VkCommandBuffer &command_buffer);
};

} // namespace inexor::vulkan_renderer
