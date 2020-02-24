#pragma once

#include <vulkan/vulkan.h>

#include <iostream>
using namespace std;


namespace inexor {
namespace vulkan_renderer {


	/// 
	/// 
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugMessageCallback(VkDebugReportFlagsEXT flags,
	                                                                 VkDebugReportObjectTypeEXT objectType,
																	 uint64_t object,
																	 size_t location,
																	 int32_t messageCode,
	                                                                 const char* pLayerPrefix,
																	 const char* pMessage,
																	 void* pUserData) 
	{
		if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		{
			spdlog::info(pMessage);
		}
		else if(flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		{
			spdlog::debug(pMessage);
		}
		else if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			spdlog::error(pMessage);
		}
		else if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		{
			spdlog::warn(pMessage);
		}
		else
		{
			spdlog::warn(pMessage);
		}

		return VK_FALSE;
	}


};
};
