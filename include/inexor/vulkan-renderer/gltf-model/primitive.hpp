#pragma once

#include "inexor/vulkan-renderer/gltf-model/bounding_box.hpp"
#include "inexor/vulkan-renderer/gltf-model/material.hpp"

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer::gltf_model {

struct Primitive {
    uint32_t first_index;

    uint32_t index_count;

    uint32_t vertex_count;

    Material &material;

    bool has_indices;

    BoundingBox bb;

    Primitive(uint32_t first_index, uint32_t index_count, uint32_t vertex_count, Material &material);

    /// @brief Sets the bounding box of the primitive.
    /// @param min [in] The minimum vector (edge of the bounding box)
    /// @param max [in] The maximum vector (edge of the bounding box)
    void set_bounding_box(glm::vec3 min, glm::vec3 max);
};

} // namespace inexor::vulkan_renderer::gltf_model
