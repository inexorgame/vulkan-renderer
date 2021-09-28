#include "inexor/vulkan-renderer/gltf/gltf_primitive.hpp"

namespace inexor::vulkan_renderer::gltf {

ModelPrimitive::ModelPrimitive(const std::uint32_t first_index, const std::uint32_t index_count,
                               const std::uint32_t vertex_count, ModelMaterial &material)
    : m_first_index(first_index), m_index_count(index_count), m_vertex_count(vertex_count), m_material(material) {
    m_has_indices = index_count > 0;
}

void ModelPrimitive::set_bbox(const glm::vec3 min, const glm::vec3 max) {
    m_bb.min = min;
    m_bb.max = max;
    m_bb.valid = true;
}

} // namespace inexor::vulkan_renderer::gltf
