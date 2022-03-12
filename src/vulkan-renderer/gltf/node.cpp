#include "inexor/vulkan-renderer/gltf/node.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>

namespace inexor::vulkan_renderer::gltf {

[[nodiscard]] glm::mat4 ModelNode::local_matrix() {
    return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) *
           matrix;
}

[[nodiscard]] glm::mat4 ModelNode::get_matrix() {
    glm::mat4 m = local_matrix();
    auto *p = parent;
    while (p) {
        m = p->local_matrix() * m;
        p = p->parent;
    }
    return m;
}

void ModelNode::update() {
    if (mesh) {
        glm::mat4 m = get_matrix();

        if (skin) {
            mesh->uniform_block.matrix = m;

            // Update joint matrices
            glm::mat4 inverseTransform = glm::inverse(m);

            std::size_t numJoints = std::min(static_cast<std::uint32_t>(skin->joints.size()), MAX_NUM_JOINTS);

            for (std::size_t i = 0; i < numJoints; i++) {
                auto *joint_node = skin->joints[i];
                glm::mat4 jointMat = joint_node->get_matrix() * skin->inverse_bind_matrices[i];
                jointMat = inverseTransform * jointMat;
                mesh->uniform_block.jointMatrix[i] = jointMat;
            }

            mesh->uniform_block.jointcount = static_cast<float>(numJoints);
            mesh->uniform_buffer->update(&mesh->uniform_block);
        } else {
            mesh->uniform_buffer->update(&m);
        }
    }

    for (auto &child : children) {
        child->update();
    }
}

} // namespace inexor::vulkan_renderer::gltf
