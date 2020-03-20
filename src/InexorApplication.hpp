#pragma once

#include "vulkan-renderer/VulkanRenderer.hpp"
#include "vulkan-renderer/shader-manager/VulkanShaderManager.hpp"
#include "vulkan-renderer/error-handling/error-handling.hpp"
#include "vulkan-renderer/tools/argument-parser/CommandLineArgumentParser.hpp"
#include "vulkan-renderer/mesh-buffer/InexorMeshBuffer.hpp"


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

			// An example texture.
			std::shared_ptr<InexorTexture> example_texture_1 = std::make_shared<InexorTexture>();
			
			// It is important to make sure that you debugging folder contains the required shader files!
			struct InexorShaderSetup
			{
				VkShaderStageFlagBits shader_type;
				std::string shader_file_name;
			};
		
			// The actual file list of shaders that we want to load.
			// TODO: Setup shaders JSON or TOML list file.
			// TODO: Implement a VulkanPipelineManager!
			const std::vector<InexorShaderSetup> shader_list = 
			{
				{VK_SHADER_STAGE_VERTEX_BIT,   "shaders/textures/vertexshader.spv"},
				{VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/textures/fragmentshader.spv"}
				// Add more shaders here..
				// TODO: Support more shader types!
			};

		private:

			VkResult load_TOML_configuration_file(const std::string& TOML_file_name);

			VkResult load_textures();

			VkResult load_shaders();

			VkResult load_models();

			VkResult draw_frame();


		public:
			
			VkResult initialise();

			void run();

			void cleanup();

	};

};
};
