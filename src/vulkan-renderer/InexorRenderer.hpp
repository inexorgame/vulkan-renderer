#pragma once

#include "shader-loading/VulkanShader.hpp"
#include "shader-loading/VulkanShaderManager.hpp"
#include "error-handling/VulkanErrorHandling.hpp"
#include "initialisation/VulkanInitialisation.hpp"


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

	};

};
};
