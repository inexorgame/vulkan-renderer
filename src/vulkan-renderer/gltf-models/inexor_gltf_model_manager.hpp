#pragma once

#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include <spdlog/spdlog.h>

#include <vulkan/vulkan.h>

#include "inexor_bounding_box.hpp"
#include "inexor_gltf_material.hpp"
#include "inexor_gltf_primitive.hpp"
#include "inexor_gltf_model_mesh.hpp"
#include "inexor_gltf_model_skin.hpp"
#include "inexor_gltf_model_animation.hpp"
#include "inexor_gltf_animation_channel.hpp"
#include "inexor_gltf_animation_sampler.hpp"
#include "inexor_texture_sampler.hpp"
#include "inexor_gltf_model_vertex.hpp"
#include "inexor_gltf_dimensions.hpp"
#include "inexor_gltf_model.hpp"

#include "../class-templates/manager_template.hpp"

#include "../texture-manager/vk_texture_manager.hpp"
#include "../uniform-buffer-manager/vk_uniform_buffer_manager.hpp"
#include "../mesh-buffer-manager/vk_mesh_buffer_manager.hpp"


// JSON for modern C++11 library.
// https://github.com/nlohmann/json
#include "nlohmann/json.hpp"
#define TINYGLTF_NO_INCLUDE_JSON

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../third_party/tiny_gltf/tiny_gltf.h"


namespace inexor {
namespace vulkan_renderer {
namespace glTF2_models {


	/// @class InexorModelManager
	/// @brief A manager class for model in glTF 2.0 format.
	/// https://www.khronos.org/gltf/
	struct InexorModelManager : ManagerClassTemplate<InexorModel>
	{

		public:
		
			InexorModelManager();

			~InexorModelManager();


		private:
		
			bool model_manager_initialised = false;
		
			std::shared_ptr<VulkanTextureManager> texture_manager;
			
			std::shared_ptr<VulkanUniformBufferManager> uniform_buffer_manager;
			
			std::shared_ptr<InexorMeshBufferManager> mesh_buffer_manager;


		protected:
		
			/// @brief Initialises Vulkan glTF 2.0 model manager.
			/// @param texture_manager [in] A shared pointer to the texture manager.
			/// @param uniform_buffer_manager [in] A shared pointer to the uniform buffer manager.
			/// @param mesh_buffer_manager [in] mesh_buffer_manager A shared pointer to the mesh buffer manager.
			VkResult initialise(const std::shared_ptr<VulkanTextureManager> texture_manager, const std::shared_ptr<VulkanUniformBufferManager> uniform_buffer_manager, const std::shared_ptr<InexorMeshBufferManager> mesh_buffer_manager);


			/// @brief Loads a glTF 2.0 file.
			/// @param internal_model_name [in] The internal name of the glTF 2.0 model which is used inside of the engine.
			/// @param glTF2_file_name [in] The filename of the glTF 2.0 file.
			VkResult load_model_from_glTF2_file(const std::string& internal_model_name, const std::string& glTF2_file_name);


			/// @brief Unloads a model and frees its memory.
			/// @param internal_model_name [in] The internal name of the glTF 2.0 model which is used inside of the engine.
			VkResult unload_model(const std::string& internal_model_name);


			/// @brief Unloads all models.
			VkResult unload_all_models();

			
			/// 
			/// 
			/// 
			VkResult draw_model(const std::string& internal_model_name, VkCommandBuffer commandBuffer);

			
			/// 
			/// 
			/// 
			VkResult draw_all_models(VkCommandBuffer commandBuffer);


		
		private:


			/// 
			void destroy();


			/// TODO: Refactor!
			void load_node(std::shared_ptr<InexorModelNode> parent, const tinygltf::Node &node, const uint32_t nodeIndex, std::shared_ptr<InexorModel> model, const float globalscale);


			/// 
			void load_skins(std::shared_ptr<InexorModel> model);


			/// 
			void load_textures(std::shared_ptr<InexorModel> model);


			/// 
			VkSamplerAddressMode get_wrap_mode(const int32_t wrapMode);


			/// 
			VkFilter get_filter_mode(const int32_t filterMode);


			/// 
			void load_texture_samplers(std::shared_ptr<InexorModel> model);


			/// 
			void load_materials(std::shared_ptr<InexorModel> model);


			/// 
			void load_animations(std::shared_ptr<InexorModel> model);


			/// 
			VkResult load_model_from_file(const std::string& file_name, const float scale = 1.0f);


			/// 
			void draw_node(std::shared_ptr<InexorModelNode> node, VkCommandBuffer commandBuffer);



			/// 
			void calculate_bounding_box(std::shared_ptr<InexorModel> model, std::shared_ptr<InexorModelNode> node, std::shared_ptr<InexorModelNode> parent);


			/// 
			void get_scene_dimensions(std::shared_ptr<InexorModel> model);
		

			/// 
			void update_animation(std::shared_ptr<InexorModel> model, const uint32_t index, const float time);


			/// 
			std::shared_ptr<InexorModelNode> find_node(std::shared_ptr<InexorModelNode> parent, const uint32_t index);


			/// 
			std::shared_ptr<InexorModelNode> node_from_index(std::shared_ptr<InexorModel> model, const uint32_t index);


	};

};
};
};
