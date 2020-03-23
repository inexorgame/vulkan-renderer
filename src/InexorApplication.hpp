#pragma once

#include "vulkan-renderer/vk_renderer.hpp"
#include "vulkan-renderer/shader-manager/vk_shader_manager.hpp"
#include "vulkan-renderer/error-handling/vk_error_handling.hpp"
#include "vulkan-renderer/tools/argument-parser/cla_parser.hpp"
#include "vulkan-renderer/mesh-buffer/vk_mesh_buffer.hpp"


// toml11: TOML for Modern C++
// https://github.com/ToruNiina/toml11
#include <toml11/toml.hpp>


using namespace inexor::vulkan_renderer::tools;

namespace inexor {
namespace vulkan_renderer {


	/// @class InexorApplication
	/// @brief The Inexor application wrapper class.
	class InexorApplication : public VulkanRenderer,
	                          public CommandLineArgumentParser
	{
		public:

			InexorApplication();
			
			~InexorApplication();


		private:
		
			// The following data will be loaded by the TOML file.
			
			std::string application_name = "";
			
			std::string engine_name = "";

			uint32_t application_version = 0;
			
			uint32_t engine_version = 0;


		private:
			
			// Frame synchronisation.
			std::size_t current_frame = 0;
			
			// The meshes (vertex buffers and index buffers).
			std::vector<InexorMeshBuffer> mesh_buffers;

			// The textures.
			std::vector<std::shared_ptr<InexorTexture>> textures;
			
			// It is important to make sure that you debugging folder contains the required shader files!
			struct InexorShaderSetup
			{
				VkShaderStageFlagBits shader_type;
				std::string shader_file_name;
			};
				
			// Vertex shaders.
			std::vector<std::string> vertex_shader_files;
			
			// Fragment shaders.
			std::vector<std::string> fragment_shader_files;

			// The texture files.
			std::vector<std::string> texture_files;

			// The shader files.
			std::vector<std::string> shader_files;

			// The glTF 2.0 model files.
			std::vector<std::string> gltf_model_files;


		private:

			VkResult load_TOML_configuration_file(const std::string& TOML_file_name);

			VkResult load_textures();

			VkResult load_shaders();

			VkResult load_models();

			VkResult check_application_specific_features();

			VkResult setup_scene();

			VkResult draw_frame();


		public:
			
			VkResult initialise();

			void run();

			void cleanup();

	};

};
};
