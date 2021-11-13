#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_bbox.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_material.hpp"

namespace inexor::vulkan_renderer::gltf {

struct ModelPrimitive {
public:
    const ModelMaterial &material;

    // TODO: Do we need first_vertex?
    std::uint32_t first_index;
    std::uint32_t index_count;
    std::uint32_t vertex_count;

    BoundingBox bbox;

    void set_bbox(const glm::vec3 &min_value, const glm::vec3 &max_value) {
        bbox.min = min_value;
        bbox.max = max_value;
        bbox.valid = true;
    }

    ModelPrimitive(std::uint32_t first_index, std::uint32_t index_count, std::uint32_t vertex_count,
                   const ModelMaterial &material);
};

} // namespace inexor::vulkan_renderer::gltf
