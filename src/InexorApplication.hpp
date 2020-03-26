#pragma once

#include "vulkan-renderer/vk_renderer.hpp"
#include "vulkan-renderer/shader-manager/vk_shader_manager.hpp"
#include "vulkan-renderer/error-handling/vk_error_handling.hpp"
#include "vulkan-renderer/tools/argument-parser/cla_parser.hpp"
#include "vulkan-renderer/keyboard/inexor_keyboard_input.hpp"
#include "vulkan-renderer/mesh-buffer/vk_mesh_buffer.hpp"
#include "vulkan-renderer/camera/InexorCamera.hpp"
#include "vulkan-renderer/debug-callback/vk_debug_callback.hpp"
#include "vulkan-renderer/uniform-buffer/vk_standard_ubo.hpp"


// toml11: TOML for Modern C++
// https://github.com/ToruNiina/toml11
#include <toml11/toml.hpp>


using namespace inexor::vulkan_renderer::tools;

namespace inexor {
namespace vulkan_renderer {


	/// @class InexorApplication
	/// @brief The Inexor application wrapper class.
	class InexorApplication : public VulkanRenderer,
	                          public InexorKeyboardInputHandler,
	                          public InexorCommandLineArgumentParser
                              // public InexorGameDataUpdater
	{
		public:

			InexorApplication();
			
			~InexorApplication();


		private:
		
			// This data will be loaded by the TOML file.
			
			std::string application_name = "";
			
			std::string engine_name = "";

			uint32_t application_version = 0;
			
			uint32_t engine_version = 0;


		private:
			
			// Frame synchronisation.
			std::size_t current_frame = 0;
			
			std::vector<InexorMeshBuffer> mesh_buffers;

			std::vector<std::shared_ptr<InexorTexture>> textures;
			
			// It is important to make sure that you debugging folder contains the required shader files!
			struct InexorShaderSetup
			{
				VkShaderStageFlagBits shader_type;
				std::string shader_file_name;
			};
				
			std::vector<std::string> vertex_shader_files;
			
			std::vector<std::string> fragment_shader_files;

			std::vector<std::string> texture_files;

			std::vector<std::string> shader_files;

			std::vector<std::string> gltf_model_files;

			InexorCamera camera;

			/// Neccesary for taking into account the relative speed of the system's CPU.
			float time_passed = 0.0f;
			
			InexorTimeStep clock_timing;



		private:

			/// @brief Loads the configuration of the renderer from a TOML configuration file.
			/// @brief TOML_file_name [in] The TOML configuration file.
			/// @note It was collectively decided not to use JSON for configuration files.
			VkResult load_TOML_configuration_file(const std::string& TOML_file_name);


			/// 
			VkResult load_textures();


			/// 
			VkResult load_shaders();


			/// 
			VkResult load_models();


			/// 
			VkResult check_application_specific_features();
			

			/// 
			VkResult draw_frame();
			

			/// 
			VkResult update_cameras();

			/// @brief Implementation of the uniform buffer update method which is defined as virtual function in VulkanRenderer.
			/// @param current_image [in] The current image index.
			VkResult update_uniform_buffer(const std::size_t current_image) override;


		public:
			
			VkResult initialise();

			
			/// @brief Keyboard input callback.
			/// @param window [in] The glfw window.
			/// @param key [in] The key which was pressed or released.
			/// @param scancode [in] The system-specific scancode of the key.
			/// @param action [in] The key action: GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT. 
			/// @param mods [in] Bit field describing which modifier keys were held down.
			void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

			
			void run();


			void cleanup();


	};

};
};
