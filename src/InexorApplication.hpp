#pragma once

#include "vulkan-renderer/VulkanRenderer.hpp"
#include "vulkan-renderer/shader-manager/VulkanShaderManager.hpp"
#include "vulkan-renderer/error-handling/VulkanErrorHandling.hpp"
#include "vulkan-renderer/tools/argument-parser/CommandLineArgumentParser.hpp"
#include "vulkan-renderer/vertex-buffer-manager/InexorMeshBuffer.hpp"


// Change these definitions if you want to fork the renderer!
#define INEXOR_ENGINE_VERSION       VK_MAKE_VERSION(1,0,0)
#define INEXOR_APPLICATION_VERSION  VK_MAKE_VERSION(1,0,0)
#define INEXOR_APPLICATION_NAME     "Inexor-Application"
#define INEXOR_ENGINE_NAME          "Inexor-Engine"
#define INEXOR_WINDOW_TITLE         "Inexor-Vulkan-Renderer"
#define INEXOR_WINDOW_WIDTH         800
#define INEXOR_WINDOW_HEIGHT        600


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
				{VK_SHADER_STAGE_VERTEX_BIT,   "vertexshader.spv"},
				{VK_SHADER_STAGE_FRAGMENT_BIT, "fragmentshader.spv"}
				// Add more shaders here..
				// TODO: Support more shader types!
			};

		private:

			VkResult load_textures();

			VkResult load_shaders();

			VkResult load_models();

			VkResult draw_frame();


		public:
			
			VkResult init();

			void run();

			void cleanup();


	};

};
};
