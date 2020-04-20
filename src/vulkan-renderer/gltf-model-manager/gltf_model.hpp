#pragma once

#include "vulkan-renderer/gltf-model-manager/gltf_model_animation.hpp"
#include "vulkan-renderer/gltf-model-manager/gltf_model_dimensions.hpp"
#include "vulkan-renderer/gltf-model-manager/gltf_model_node.hpp"
#include "vulkan-renderer/gltf-model-manager/gltf_model_texture_sampler.hpp"
#include "vulkan-renderer/gltf-model-manager/gltf_model_vertex.hpp"
#include "vulkan-renderer/mesh-buffer/mesh_buffer.hpp"
#include "vulkan-renderer/texture/texture.hpp"

// Header only C++ tiny glTF library(loader/saver).
// https://github.com/syoyo/tinygltf
// License: MIT.
#include <tiny_gltf/tiny_gltf.h>

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace inexor {
namespace vulkan_renderer {

struct InexorModel {
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

    std::size_t uniform_buffer_index = 0;
};

}; // namespace vulkan_renderer
}; // namespace inexor
