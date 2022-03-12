#include "inexor/vulkan-renderer/gltf/mesh.hpp"

namespace inexor::vulkan_renderer::gltf {

ModelMesh::ModelMesh(const wrapper::Device &device, const glm::mat4 matrix) {
    uniform_block.matrix = matrix;
    uniform_buffer = std::make_unique<wrapper::UniformBuffer<UniformBlock>>(device, "gltf ubo");
    uniform_buffer->update(&uniform_block);
}

} // namespace inexor::vulkan_renderer::gltf
