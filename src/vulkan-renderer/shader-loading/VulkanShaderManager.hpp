#pragma once

#include <vector>
#include <string>
#include <iostream>
using namespace std;

#include <vulkan/vulkan.h>

#include "../error-handling/VulkanErrorHandling.hpp"
#include "VulkanShader.hpp"


namespace inexor {
namespace vulkan_renderer {

	// 
	class VulkanShaderManager
	{
		public:

			// 
			VulkanShaderManager();
			
			// 
			~VulkanShaderManager();

		protected:

			// 
			void create_shader_module(const VkDevice& vulkan_device, const std::vector<char>& SPIRV_shader_bytes, VkShaderModule* shader_module);

			// 
			void create_shader_module_from_file(const VkDevice& vulkan_device, const std::string& SPIRV_file_name, VkShaderModule* shader_module);

	};


};
};
