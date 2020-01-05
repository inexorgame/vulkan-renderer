#pragma once

#include "shader-loading/VulkanShader.hpp"
#include "shader-loading/VulkanShaderManager.hpp"
#include "error-handling/VulkanErrorHandling.hpp"
#include "initialisation/VulkanInitialisation.hpp"


// Change these definitions if you want to fork the renderer!
// These definitions will be used in the create_vulkan_instance() method.
#define INEXOR_ENGINE_VERSION      VK_MAKE_VERSION(1,0,0)
#define INEXOR_APPLICATION_VERSION VK_MAKE_VERSION(1,0,0)
#define INEXOR_APPLICATION_NAME    "Inexor-Application"
#define INEXOR_ENGINE_NAME         "Inexor-Engine"
#define INEXOR_APP_WINDOW_TITLE    "Inexor-Vulkan-Renderer"
#define INEXOR_WINDOW_WIDTH        800
#define INEXOR_WINDOW_HEIGHT       600


namespace inexor {
namespace vulkan_renderer {

	// TODO: VulkanTextureManager
	// TODO: VulkanGLTFModelManager

	// Now I am become renderer, the drawer of worlds.
	class InexorRenderer : public VulkanInitialisation,
						   public VulkanShaderManager
	{
		public:

			InexorRenderer();
			
			~InexorRenderer();


		private:

			// Load all required shaders.
			VkResult load_shaders();
			
			// The actual rendering method which is called every frame.
			VkResult draw_frame();


		public:

			// Initialise the Vulkan renderer.
			void init();

			// Run the event loop of the Vulkan renderer.
			void run();

			// Destroy the window and shutdown Vulkan.
			void cleanup();
			
			// React to window resize.
			void on_window_resized();

	};

};
};
