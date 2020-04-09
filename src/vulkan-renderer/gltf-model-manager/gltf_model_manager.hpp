#pragma once

#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include "gltf_model_bounding_box.hpp"
#include "gltf_model_material.hpp"
#include "gltf_model_primitive.hpp"
#include "gltf_model_mesh.hpp"
#include "gltf_model_skin.hpp"
#include "gltf_model_animation.hpp"
#include "gltf_model_animation_channel.hpp"
#include "gltf_model_animation_sampler.hpp"
#include "gltf_model_texture_sampler.hpp"
#include "gltf_model_vertex.hpp"
#include "gltf_model_dimensions.hpp"
#include "gltf_model.hpp"

#include "../class-templates/manager_template.hpp"
#include "../texture-manager/texture_manager.hpp"
#include "../uniform-buffer-manager/uniform_buffer_manager.hpp"
#include "../mesh-buffer-manager/mesh_buffer_manager.hpp"
#include "../descriptor-manager/descriptor_manager.hpp"


// JSON for modern C++11 library.
// https://github.com/nlohmann/json
#include "nlohmann/json.hpp"
#define TINYGLTF_NO_INCLUDE_JSON


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorModelManager
	/// TODO: Make InexorModelLoader and inherit!
	/// @brief A manager class for model in glTF 2.0 format.
	/// https://www.khronos.org/gltf/
	struct InexorModelManager : ManagerClassTemplate<InexorModel>
	{

		public:
		
			InexorModelManager() = default;

			~InexorModelManager() = default;


		private:
		
			VkDevice device;
			
			bool model_manager_initialised = false;
		
			std::shared_ptr<VulkanTextureManager> texture_manager;
			
			std::shared_ptr<VulkanUniformBufferManager> uniform_buffer_manager;
			
			std::shared_ptr<InexorMeshBufferManager> mesh_buffer_manager;

			std::shared_ptr<InexorDescriptorManager> descriptor_manager;

			// The global descriptor bundle for glTF 2.0 models.
			std::shared_ptr<InexorDescriptorBundle> gltf_global_descriptor_bundle;

		
		public:
		
			/// @brief Initialises Vulkan glTF 2.0 model manager.
			/// @param device [in] The Vulkan device.
			/// @param global_descriptor_bundle [in] The global descriptor bundle.
			/// @param texture_manager [in] A shared pointer to the texture manager.
			/// @param uniform_buffer_manager [in] A shared pointer to the uniform buffer manager.
			/// @param mesh_buffer_manager [in] mesh_buffer_manager A shared pointer to the mesh buffer manager.
			VkResult initialise(const VkDevice& device,
								const std::shared_ptr<VulkanTextureManager> texture_manager,
			                    const std::shared_ptr<VulkanUniformBufferManager> uniform_buffer_manager,
								const std::shared_ptr<InexorMeshBufferManager> mesh_buffer_manager,
								const std::shared_ptr<InexorDescriptorManager> descriptor_manager);


			/// @brief Loads a glTF 2.0 file.
			/// @param internal_model_name [in] The internal name of the glTF 2.0 model which is used inside of the engine.
			/// @param glTF2_file_name [in] The filename of the glTF 2.0 file.
			VkResult load_model_from_glTF2_file(const std::string& internal_model_name,
			                                    const std::string& glTF2_file_name);

			
			/// @brief Renders a certain model during the recording of a command buffer.
			/// @param internal_model_name [in] The internal name of the glTF 2.0 model.
			/// The model must be loaded at this point already.
			/// @param command_buffer[in] The command buffer which is being recorded.
			/// @param pipeline_layout [in] The pipeline layout.
			/// @param current_image_index [in] The current frame index.
			VkResult render_model(const std::string& internal_model_name,
			                      VkCommandBuffer command_buffer,
								  VkPipelineLayout pipeline_layout,
								  std::size_t current_image_index);

			
			/// @brief Renders all models during the recording of a command buffer.
			/// @param command_buffer[in] The command buffer which is being recorded.
			/// @param pipeline_layout [in] The pipeline layout.
			/// @param current_image_index [in] The current frame index.
			VkResult render_all_models(VkCommandBuffer command_buffer,
									   VkPipelineLayout pipeline_layout,
									   std::size_t current_image_index);


			/// 
			VkResult create_model_descriptors(const std::size_t number_of_images_in_swapchain);


			/// @brief Returns the number of existing models.
			std::size_t get_model_count();
		
			
		private:

			
			/// @brief Sets up descriptor sets for glTF model nodes.
			/// @param node [in] A glTF model node.
			VkResult setup_node_descriptor_set(std::shared_ptr<InexorModelNode> node);


			/// 
			/// 
			/// 
			VkResult load_model_from_file(const std::string& file_name, std::shared_ptr<InexorModel>& new_model, const float scale = 1.0f);

			
			/// 
			void destroy();


			/// 
			void load_node(std::shared_ptr<InexorModelNode> parent, 
						   const tinygltf::Node &node,
						   const uint32_t nodeIndex,
						   std::shared_ptr<InexorModel> model,
						   const float globalscale);


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
			void render_node(std::shared_ptr<InexorModelNode> node, VkCommandBuffer commandBuffer, VkPipelineLayout pipeline_layout, std::size_t current_image_index);


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
