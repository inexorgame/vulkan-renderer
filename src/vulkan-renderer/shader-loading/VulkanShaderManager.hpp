#pragma once

#include "../error-handling/VulkanErrorHandling.hpp"
#include "../shader-loading/VulkanShader.hpp"

#include <vector>
#include <string>
#include <iostream>


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanShaderManager
	/// @brief A class for managing SPIR-V shaders.
	class VulkanShaderManager
	{
		public:

			VulkanShaderManager();
			
			~VulkanShaderManager();

		
		private:

			// TODO: Implement!
			std::vector<VkShaderModule> shaders;
			
		protected:

			/// @brief Creates a shader module.
			/// @param vulkan_device The Vulkan device handle.
			/// @param SPIRV_shader_bytes The binary data of the shader.
			/// @param shader_module The shader module.
			/// @note The buffer with the SPIR-V code can be freed immediately after the shader module was created.
			VkResult create_shader_module(const VkDevice& vulkan_device, const std::vector<char>& SPIRV_shader_bytes, VkShaderModule* shader_module);

			
			/// @brief Creates a shader module from a SPIR-V shader file.
			/// @param vulkan_device The Vulkan device handle.
			/// @param SPIRV_file_name The name of the SPIR-V shader file.
			/// @param shader_module The shader module.
			/// @note The buffer with the SPIR-V code can be freed immediately after the shader module was created.
			VkResult create_shader_module_from_file(const VkDevice& vulkan_device, const std::string& SPIRV_file_name, VkShaderModule* shader_module);

	};

};
};
