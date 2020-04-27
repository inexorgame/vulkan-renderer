#include "inexor/vulkan-renderer/gltf-model/mesh.hpp"

namespace inexor::vulkan_renderer::gltf_model {
void Mesh::set_matrix(const glm::mat4 &mat) {
    uniform_block.matrix = mat;
}

void Mesh::set_bounding_box(glm::vec3 min, glm::vec3 max) {
    bb.min = min;
    bb.max = max;
    bb.valid = true;
}
}
