#include "inexor/vulkan-renderer/gltf/mesh.hpp"

namespace inexor::vulkan_renderer::gltf {

ModelMesh::ModelMesh(const wrapper::Device &device, const glm::mat4 matrix) {
    uniformBlock.matrix = matrix;
    uniform_buffer = std::make_unique<wrapper::UniformBuffer<UniformBlock>>(device, "gltf ubo");
}

} // namespace inexor::vulkan_renderer::gltf
