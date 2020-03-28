#pragma once

#include "inexor_gltf_dimensions.hpp"
#include "inexor_gltf_model_node.hpp"
#include "inexor_gltf_model_animation.hpp"

#include "../texture/vk_texture.hpp"

#include "../mesh-buffer/vk_mesh_buffer.hpp"


// JSON for modern C++11 library.
// https://github.com/nlohmann/json
#include "nlohmann/json.hpp"
#define TINYGLTF_NO_INCLUDE_JSON

#include "../../third_party/tiny_gltf/tiny_gltf.h"

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>


namespace inexor {
namespace vulkan_renderer {
namespace glTF2_models {


	struct InexorModel
	{
		tinygltf::Model gltf2_container;

		std::string name = "";
		
		glm::mat4 aabb;
		
		std::vector<uint32_t> index_buffer_cache;
		
		std::vector<InexorModelVertex> vertex_buffer_cache;

		std::shared_ptr<InexorMeshBuffer> mesh;

		std::vector<std::shared_ptr<InexorModelNode>> nodes;

		std::vector<std::shared_ptr<InexorModelNode>> linear_nodes;

		std::vector<std::shared_ptr<InexorModelSkin>> skins;

		std::vector<std::shared_ptr<InexorTexture>> textures;

		std::vector<InexorTextureSampler> texture_samplers;

		std::vector<InexorModelMaterial> materials;

		std::vector<InexorModelAnimation> animations;

		std::vector<std::string> extensions;

		InexorDimensions dimensions;
	};


};
};
};
