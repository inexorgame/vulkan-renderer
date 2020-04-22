#include "inexor/vulkan-renderer/debug_marker_manager.hpp"

namespace inexor::vulkan_renderer {

void VulkanDebugMarkerManager::init(const VkDevice &device, const VkPhysicalDevice &graphics_card, bool enable_debug_markers) {
    if (enable_debug_markers) {
        // Check if the debug marker extension is present (which is the case if run from a graphics debugger)
        uint32_t extensionCount;

        vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);

        vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &extensionCount, extensions.data());
        for (const auto &extension : extensions) {
            if (0 == strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
                active = true;
                extension_present = true;
                break;
            }
        }

        if (extension_present) {
            // The debug marker extension is not part of the core, so function pointers need to be loaded manually
            vkDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
            assert(vkDebugMarkerSetObjectTag);

            vkDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
            assert(vkDebugMarkerSetObjectName);

            vkCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
            assert(vkCmdDebugMarkerBegin);

            vkCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
            assert(vkCmdDebugMarkerEnd);

            vkCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
            assert(vkCmdDebugMarkerInsert);

            // Set flag if at least one function pointer is present
            active = true;
        } else {
            spdlog::warn("Warning: {} not present, debug markers are disabled.", VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
            spdlog::warn("Try running from inside a Vulkan graphics debugger (e.g. RenderDoc).");
        }
    }
}

void VulkanDebugMarkerManager::set_object_name(const VkDevice &device, const uint64_t &object, const VkDebugReportObjectTypeEXT &object_type,
                                               const char *name) {
    assert(device);
    assert(name);
    assert(object);

    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (active) {
        VkDebugMarkerObjectNameInfoEXT nameInfo = {};

        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = object_type;
        nameInfo.object = object;
        nameInfo.pObjectName = name;

        assert(vkDebugMarkerSetObjectName);
        vkDebugMarkerSetObjectName(device, &nameInfo);
    }
}

void VulkanDebugMarkerManager::set_object_tag(const VkDevice &device, const uint64_t &object, const VkDebugReportObjectTypeEXT &object_type,
                                              const uint64_t &name, const std::size_t &tag_size, const void *tag) {
    assert(device);

    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (active) {
        VkDebugMarkerObjectTagInfoEXT tagInfo = {};

        tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
        tagInfo.objectType = object_type;
        tagInfo.object = object;
        tagInfo.tagName = name;
        tagInfo.tagSize = tag_size;
        tagInfo.pTag = tag;

        assert(vkDebugMarkerSetObjectTag);
        vkDebugMarkerSetObjectTag(device, &tagInfo);
    }
}

void VulkanDebugMarkerManager::bind_region(const VkCommandBuffer &command_buffer, const std::string &debug_marker_name, const glm::vec4 &debug_marker_color) {
    assert(command_buffer);
    assert(!debug_marker_name.empty());

    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (active) {
        VkDebugMarkerMarkerInfoEXT debug_marker_info = {};

        debug_marker_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        debug_marker_info.color[0] = debug_marker_color[0];
        debug_marker_info.color[1] = debug_marker_color[1];
        debug_marker_info.color[2] = debug_marker_color[2];
        debug_marker_info.color[3] = debug_marker_color[3];
        debug_marker_info.pMarkerName = debug_marker_name.c_str();

        assert(vkCmdDebugMarkerBegin);
        vkCmdDebugMarkerBegin(command_buffer, &debug_marker_info);
    }
}

void VulkanDebugMarkerManager::insert(const VkCommandBuffer &command_buffer, const std::string &debug_marker_name, const glm::vec4 &debug_marker_color) {
    assert(command_buffer);
    assert(!debug_marker_name.empty());

    // Check for valid function pointer (may not be present if not running in a debugging application)
    if (active) {
        VkDebugMarkerMarkerInfoEXT debug_marker_info = {};

        debug_marker_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
        debug_marker_info.color[0] = debug_marker_color[0];
        debug_marker_info.color[1] = debug_marker_color[1];
        debug_marker_info.color[2] = debug_marker_color[2];
        debug_marker_info.color[3] = debug_marker_color[3];
        debug_marker_info.pMarkerName = debug_marker_name.c_str();

        assert(vkCmdDebugMarkerInsert);
        vkCmdDebugMarkerInsert(command_buffer, &debug_marker_info);
    }
}

void VulkanDebugMarkerManager::end_region(const VkCommandBuffer &command_buffer) {
    // Check for valid function (may not be present if not runnin in a debugging application)
    if (vkCmdDebugMarkerEnd) {
        assert(vkCmdDebugMarkerEnd);
        vkCmdDebugMarkerEnd(command_buffer);
    }
}

} // namespace inexor::vulkan_renderer
