#pragma once

#include "inexor/vulkan-renderer/gltf/bounding_box.hpp"
#include "inexor/vulkan-renderer/gltf/model_material.hpp"

namespace inexor::vulkan_renderer::gltf {

/// @brief A struct for glTF2 model primitives.
struct ModelPrimitive {
    std::uint32_t m_first_index;
    std::uint32_t m_index_count;
    std::uint32_t m_vertex_count;
    const ModelMaterial &m_material;
    bool m_has_indices{false};
    BoundingBox m_bb;

    ///
    ///
    ///
    ///
    ///
    ModelPrimitive(const std::uint32_t firstIndex, const std::uint32_t indexCount, const std::uint32_t vertexCount,
                   ModelMaterial &material);

    [[nodiscard]] std::uint32_t index_count() const {
        return m_index_count;
    }

    ///
    ///
    ///
    void set_bbox(const glm::vec3 min, const glm::vec3 max);
};

} // namespace inexor::vulkan_renderer::gltf
