#pragma once

#include "shader-manager/VulkanShaderManager.hpp"
#include "error-handling/VulkanErrorHandling.hpp"
#include "initialisation/VulkanRenderer.hpp"
#include "tools/argument-parser/CommandLineArgumentParser.hpp"
#include "vertex-buffer-manager/InexorMeshBuffer.hpp"

#include <spdlog/spdlog.h>


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


	/// @class InexorRenderer
	/// @brief Inexor's Vulkan API rendering engine.
	/// @note Now I am become renderer, the drawer of worlds.
	class InexorApplication : public VulkanRenderer, 
	                          public CommandLineArgumentParser
	{
		public:

			InexorApplication();
			
			~InexorApplication();


		private:
			
			// 
			std::size_t current_frame = 0;
			
			// 
			std::vector<InexorMeshBuffer> mesh_buffers;

			// 
			std::shared_ptr<InexorTexture> example_texture = std::make_shared<InexorTexture>();


		private:

			/// @brief Load all required shaders.
			VkResult load_shaders();

			/// @brief Loads all required models.
			VkResult load_models();

			/// @brief The actual rendering method which is called every frame.
			VkResult draw_frame();


		public:
			
			/// @brief Initialise the Vulkan renderer.
			VkResult init();

			/// @brief Run the event loop of the Vulkan renderer.
			void run();

			/// @brief Destroy the window and shutdown Vulkan.
			void cleanup();


	};

};
};
