#include "inexor/vulkan-renderer/gltf/primitive.hpp"

namespace inexor::vulkan_renderer::gltf {

ModelPrimitive::ModelPrimitive(const std::uint32_t first_index, const std::uint32_t index_count,
                               const std::uint32_t vertex_count, const ModelMaterial &model_material)
    : material(model_material) {

    this->first_index = first_index;
    this->index_count = index_count;
    this->vertex_count = vertex_count;
}

} // namespace inexor::vulkan_renderer::gltf
