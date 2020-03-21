#pragma once

#include "../tools/file-loader/file.hpp"

#include <vulkan/vulkan.h>
#include <string>


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorVulkanShader
	/// @brief A class which bundles information about shaders.
	class InexorVulkanShader : public tools::InexorFile
	{
		private:

			// The name of the shader.
			std::string name;
		
			// This information will be passed directly to VkPipelineShaderStageCreateInfo::stage.
			VkShaderStageFlagBits type;
		
			// The entry point of the shader program, usually "main".
			std::string entry_name;
		
			// The shader module.
			VkShaderModule vulkan_shader_module;

		public:

			InexorVulkanShader();

			~InexorVulkanShader();

			
			/// @brief Sets the name of the shader.
			/// @param shader_name The name of the shader.
			void set_shader_name(const std::string& shader_name);

			/// @brief Sets the entry point's name of the shader.
			/// @param shader_name The entry point's name of the shader.
			void set_shader_entry_point(const std::string& entry_point_name);

			/// @brief Specifies the type of this shader (vertex, fragment, geometry.. shader?).
			/// @param shader_type The shader type.
			void set_shader_type(const VkShaderStageFlagBits& shader_type);
			
			/// @brief Specifies the shader module of this shader.
			/// @param shader_module The shader module.
			void set_shader_module(const VkShaderModule& shader_module);

			
			/// @brief Returns the name of the shader.
			const std::string get_shader_name() const;

			/// @brief Returns the entry point's name of the shader.
			const std::string get_shader_entry_point() const;

			/// @brief Returns the type of the shader.
			const VkShaderStageFlagBits get_shader_type() const;
			
			/// @brief Returns the shader module.
			const VkShaderModule get_shader_module() const;

	};


};
};
