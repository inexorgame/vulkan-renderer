#pragma once

#include "inexor/vulkan-renderer/gltf-model/bounding_box.hpp"
#include "inexor/vulkan-renderer/gltf-model/material.hpp"

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer::gltf_model {

struct InexorModelPrimitive {
    uint32_t first_index;

    uint32_t index_count;

    uint32_t vertex_count;

    InexorModelMaterial &material;

    bool hasIndices;

    BoundingBox bb;

    ///
    ///
    ///
    InexorModelPrimitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, InexorModelMaterial &material)
        : first_index(firstIndex), index_count(indexCount), vertex_count(vertexCount), material(material) {
        hasIndices = indexCount > 0;
    };

    /// @brief Sets the bounding box of the primitive.
    /// @param min [in] The minimum vector (edge of the bounding box)
    /// @param max [in] The maximum vector (edge of the bounding box)
    void set_bounding_box(glm::vec3 min, glm::vec3 max) {
        bb.min = min;
        bb.max = max;
        bb.valid = true;
    }
};

} // namespace inexor::vulkan_renderer::gltf_model
