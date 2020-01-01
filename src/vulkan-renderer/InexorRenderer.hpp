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

	// 
	class InexorRenderer : public VulkanInitialisation,
						   public VulkanShaderManager
	{
		private:

			// 
			void load_shaders();

			// 
			void draw_frame();
			
			// 
			void prepare_drawing();

		private:

			// 
			VkSubmitInfo submit_info;
			
			// 
			VkPipelineStageFlags wait_stage_mask[1];
			
			// 
			VkPresentInfoKHR present_info = {};


		public:
			
			// 
			InexorRenderer();
			
			// 
			~InexorRenderer();

			// 
			void init();

			// 
			void run();

			// 
			void cleanup();
			
			// 
			void on_window_resized();

	};

};
};
