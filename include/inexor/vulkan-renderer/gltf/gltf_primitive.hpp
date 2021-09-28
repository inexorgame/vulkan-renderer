#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_bbox.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_material.hpp"

namespace inexor::vulkan_renderer::gltf {

/// @brief A struct for glTF2 model primitives.
class ModelPrimitive {
private:
    std::uint32_t m_first_index;
    std::uint32_t m_index_count;
    std::uint32_t m_vertex_count;
    const ModelMaterial &m_material;
    bool m_has_indices{false};
    BoundingBox m_bb;

public:
    ///
    ///
    ///
    ///
    ///
    ModelPrimitive(std::uint32_t first_index, std::uint32_t index_count, std::uint32_t vertex_count,
                   ModelMaterial &material);

    [[nodiscard]] std::uint32_t index_count() const {
        return m_index_count;
    }

    [[nodiscard]] std::uint32_t first_index() const {
        return m_first_index;
    }

    ///
    ///
    ///
    void set_bbox(glm::vec3 min, glm::vec3 max);
};

} // namespace inexor::vulkan_renderer::gltf
