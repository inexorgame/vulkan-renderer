#include "inexor/vulkan-renderer/gltf-model/primitive.hpp"

namespace inexor::vulkan_renderer::gltf_model {
Primitive::Primitive(std::uint32_t first_index, std::uint32_t index_count, std::uint32_t vertex_count, Material &material)
    : first_index(first_index), index_count(index_count), vertex_count(vertex_count), material(material) {
    has_indices = index_count > 0;
}

void Primitive::set_bounding_box(glm::vec3 min, glm::vec3 max) {
    bb.min = min;
    bb.max = max;
    bb.valid = true;
}
} // namespace inexor::vulkan_renderer::gltf_model
