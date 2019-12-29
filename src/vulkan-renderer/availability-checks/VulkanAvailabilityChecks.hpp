#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include <vulkan/vulkan.h>

#include "../error-handling/VulkanErrorHandling.hpp"


namespace inexor {
namespace vulkan_renderer {

	// 
	class VulkanAvailabilityChecks
	{
		public:

			VulkanAvailabilityChecks();

			~VulkanAvailabilityChecks();

		protected:
		
			// 
			bool CheckInstanceExtensionSupport(const std::string& instance_extension_name);

			// 
			bool CheckInstanceLayerSupport(const std::string& instance_layer_name);

	};

};
};
