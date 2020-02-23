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
		// TODO: Embedd spdlog.
		cout << pMessage << endl;
		return VK_FALSE;
	}


};
};
