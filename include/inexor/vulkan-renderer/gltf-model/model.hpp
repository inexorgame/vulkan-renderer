#pragma once

#include "inexor/vulkan-renderer/gltf-model/animation.hpp"
#include "inexor/vulkan-renderer/gltf-model/dimensions.hpp"
#include "inexor/vulkan-renderer/gltf-model/node.hpp"
#include "inexor/vulkan-renderer/gltf-model/texture_sampler.hpp"
#include "inexor/vulkan-renderer/gltf-model/vertex.hpp"
#include "inexor/vulkan-renderer/mesh_buffer.hpp"
#include "inexor/vulkan-renderer/texture.hpp"

#include <glm/glm.hpp>
#include <tiny_gltf/tiny_gltf.h>

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::gltf_model {

struct Model {
    tinygltf::Model gltf2_container;

    std::string name = "";

    glm::mat4 aabb;

    std::vector<std::uint32_t> index_buffer_cache;

    std::vector<ModelVertex> vertex_buffer_cache;

    std::shared_ptr<MeshBuffer> mesh;

    std::vector<std::shared_ptr<ModelNode>> nodes;

    std::vector<std::shared_ptr<ModelNode>> linear_nodes;

    std::vector<std::shared_ptr<ModelSkin>> skins;

    std::vector<std::shared_ptr<Texture>> textures;

    std::vector<TextureSampler> texture_samplers;

    std::vector<Material> materials;

    std::vector<Animation> animations;

    std::vector<std::string> extensions;

    Dimensions dimensions;

    std::size_t uniform_buffer_index = 0;
};

} // namespace inexor::vulkan_renderer::gltf_model
