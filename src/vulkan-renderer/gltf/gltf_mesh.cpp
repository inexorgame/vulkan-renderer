#include "inexor/vulkan-renderer/gltf/gltf_mesh.hpp"

namespace inexor::vulkan_renderer::gltf {

ModelMesh::ModelMesh(const wrapper::Device &device, const glm::mat4 matrix) {
    uniformBlock.matrix = matrix;
    ubo = std::make_unique<wrapper::UniformBuffer>(device, "gltf ubo", sizeof(uniformBlock));
}

} // namespace inexor::vulkan_renderer::gltf
