#include "inexor/vulkan-renderer/gltf-model/primitive.hpp"

namespace inexor::vulkan_renderer::gltf_model {
Primitive::Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material &material)
    : first_index(firstIndex), index_count(indexCount), vertex_count(vertexCount), material(material) {
    hasIndices = indexCount > 0;
}

void Primitive::set_bounding_box(glm::vec3 min, glm::vec3 max) {
    bb.min = min;
    bb.max = max;
    bb.valid = true;
}
} // namespace inexor::vulkan_renderer::gltf_model
