#pragma once


#include "../error-handling/error-handling.hpp"
#include "../shader-manager/InexorVulkanShader.hpp"
#include "../debug-marker/VulkanDebugMarkerManager.hpp"


#include <vector>
#include <string>

#include <spdlog/spdlog.h>


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
			

			// The debug marker manager.
			std::shared_ptr<VulkanDebugMarkerManager> dbg_marker_manager;


			/// The shaders which have been loaded into memory.
			// TODO: Refactor this into an unordered_map if neccesary?
			std::vector<InexorVulkanShader> shaders;


		private:


			/// @brief Creates a shader module.
			/// @param vulkan_device The Vulkan device handle.
			/// @param SPIRV_shader_bytes The binary data of the shader.
			/// @param shader_module The shader module.
			/// @note The buffer with the SPIR-V code can be freed immediately after the shader module was created.
			VkResult create_shader_module(const VkDevice& vulkan_device, const std::vector<char>& SPIRV_shader_bytes, VkShaderModule* shader_module);


		protected:


			/// @param debug_marker_manager_instance The VulkanDebugMarkerManager instance.
			void initialise(const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager_instance);


			/// @brief Destroy all shader objects.
			/// @param vulkan_device The Vulkan device.
			void shutdown_shaders(const VkDevice& vulkan_device);

			
			/// @brief Creates a new shader from SPIR-V byte buffer.
			/// @param vulkan_device The Vulkan device.
			/// @param shader_type The shader type (e.g. vertex-, fragment-, tesselation-, geometry-, compute-shader)
			/// @param SPIRV_shader_bytes The byte buffer of the SPIR-V shader file.
			/// @param shader_name The name you want to give to the shader.
			/// @param shader_entry_point The entry point of the shader, usually "main".
			/// @return True is shader creation succeeded, false otherwise.
			VkResult create_shader_from_byte_buffer(const VkDevice& vulkan_device, const VkShaderStageFlagBits& shader_type, const std::vector<char>& SPIRV_shader_bytes, const std::string& shader_name = "", const std::string& entry_point = "main");

			
			/// @brief Creates a new shader from a file on the hard drive.
			/// @param vulkan_device The Vulkan device.
			/// @param shader_type The shader type (e.g. vertex-shader, fragment-shader, tesselation-shader, geometry-shader, compute-shader)
			/// @param SPIRV_shader_file_name The file name of the SPIR-V shader file.
			/// @param shader_name The name you want to give to the shader.
			/// @param shader_entry_point The entry point of the shader, usually "main".
			/// @return True is shader creation succeeded, false otherwise.
			VkResult create_shader_from_file(const VkDevice& vulkan_device, const VkShaderStageFlagBits& shader_type, const std::string& SPIRV_shader_file_name, const std::string& shader_name = "", const std::string& shader_entry_point = "main");


			// TODO: Add overloaded methods for creation of specific shaders: create_vertex_shader...
			

			/// @brief Returns all the shaders which have been loaded.
			/// @return A const vector of InexorVulkanShader instances.
			const std::vector<InexorVulkanShader> get_shaders() const;


	};

};
};
