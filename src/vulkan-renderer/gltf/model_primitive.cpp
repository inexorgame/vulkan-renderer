#include "inexor/vulkan-renderer/gltf/model_primitive.hpp"

namespace inexor::vulkan_renderer::gltf {

ModelPrimitive::ModelPrimitive(const std::uint32_t firstIndex, const std::uint32_t indexCount,
                               const std::uint32_t vertexCount, ModelMaterial &material)
    : m_first_index(firstIndex), m_index_count(indexCount), m_vertex_count(vertexCount), m_material(material) {
    m_has_indices = indexCount > 0;
}

void ModelPrimitive::set_bbox(const glm::vec3 min, const glm::vec3 max) {
    m_bb.min = min;
    m_bb.max = max;
    m_bb.valid = true;
}

} // namespace inexor::vulkan_renderer::gltf
