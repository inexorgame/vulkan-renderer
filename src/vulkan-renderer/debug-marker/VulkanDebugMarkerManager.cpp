#include "VulkanDebugMarkerManager.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	VulkanDebugMarkerManager::VulkanDebugMarkerManager()
	{
	}

	
	VulkanDebugMarkerManager::~VulkanDebugMarkerManager()
	{
	}
	

	VkResult VulkanDebugMarkerManager::initialise(const VkDevice& device, const VkPhysicalDevice& graphics_card)
	{
		// Check if the debug marker extension is present (which is the case if run from a graphics debugger)
		uint32_t extensionCount;

		vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &extensionCount, nullptr);
				
		std::vector<VkExtensionProperties> extensions(extensionCount);

		vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &extensionCount, extensions.data());
		for(const auto& extension : extensions)
		{
			if(0 == strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
			{
				active = true;
				break;
			}
		}

		if(extension_present)
		{
			// The debug marker extension is not part of the core, so function pointers need to be loaded manually
			vkDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");

			vkDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
					
			vkCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
					
			vkCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
					
			vkCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");

			// Set flag if at least one function pointer is present
			active = (vkDebugMarkerSetObjectName != VK_NULL_HANDLE);

			return VK_SUCCESS;
		}
		else
		{
			std::cout << "Warning: " << VK_EXT_DEBUG_MARKER_EXTENSION_NAME << " not present, debug markers are disabled.";
			std::cout << "Try running from inside a Vulkan graphics debugger (e.g. RenderDoc)" << std::endl;
		}

		return VK_ERROR_INITIALIZATION_FAILED;
	}


	void VulkanDebugMarkerManager::set_object_name(const VkDevice& device, const uint64_t& object, const VkDebugReportObjectTypeEXT& object_type, const char *name)
	{
		assert(extension_present);

		// Check for valid function pointer (may not be present if not running in a debugging application)
		if(active)
		{
			VkDebugMarkerObjectNameInfoEXT nameInfo = {};

			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = object_type;
			nameInfo.object = object;
			nameInfo.pObjectName = name;

			vkDebugMarkerSetObjectName(device, &nameInfo);
		}
	}


	void VulkanDebugMarkerManager::set_object_tag(const VkDevice& device, const uint64_t& object, const VkDebugReportObjectTypeEXT& object_type, const uint64_t& name, const std::size_t& tag_size, const void* tag)
	{
		assert(extension_present);

		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (active)
		{
			VkDebugMarkerObjectTagInfoEXT tagInfo = {};

			tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
			tagInfo.objectType = object_type;
			tagInfo.object = object;
			tagInfo.tagName = name;
			tagInfo.tagSize = tag_size;
			tagInfo.pTag = tag;

			vkDebugMarkerSetObjectTag(device, &tagInfo);
		}
	}


	void VulkanDebugMarkerManager::bind_region(const VkCommandBuffer& cmdbuffer, const std::string& debug_marker_name, const glm::vec4& color)
	{
		assert(extension_present);

		// Check for valid function pointer (may not be present if not running in a debugging application)
		if (active)
		{
			VkDebugMarkerMarkerInfoEXT markerInfo = {};

			markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(markerInfo.color, &color[0], sizeof(float) * 4);

			markerInfo.pMarkerName = debug_marker_name.c_str();
			vkCmdDebugMarkerBegin(cmdbuffer, &markerInfo);
		}
	}


	void VulkanDebugMarkerManager::insert(const VkCommandBuffer& command_buffer, const std::string& debug_marker_name, glm::vec4 debug_marker_color)
	{
		assert(extension_present);
		assert(debug_marker_name.length()>0);

		// Check for valid function pointer (may not be present if not running in a debugging application)
		if(active)
		{
			VkDebugMarkerMarkerInfoEXT debug_marker_info = {};
					
			debug_marker_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			memcpy(debug_marker_info.color, &debug_marker_color[0], sizeof(float) * 4);
			debug_marker_info.pMarkerName = debug_marker_name.c_str();

			vkCmdDebugMarkerInsert(command_buffer, &debug_marker_info);
		}
	}


	void VulkanDebugMarkerManager::end_region(const VkCommandBuffer& command_buffer)
	{
		assert(extension_present);

		// Check for valid function (may not be present if not runnin in a debugging application)
		if(vkCmdDebugMarkerEnd)
		{
			vkCmdDebugMarkerEnd(command_buffer);
		}
	}


};
};
