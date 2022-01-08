#include "inexor/vulkan-renderer/gltf/node.hpp"

#include <algorithm>

namespace inexor::vulkan_renderer::gltf {

glm::mat4 ModelNode::local_matrix() {
    return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) *
           matrix;
}

glm::mat4 ModelNode::get_matrix() {
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
            mesh->uniformBlock.matrix = m;

            // Update joint matrices
            glm::mat4 inverseTransform = glm::inverse(m);

            std::size_t numJoints = std::min(static_cast<std::uint32_t>(skin->joints.size()), MAX_NUM_JOINTS);

            for (std::size_t i = 0; i < numJoints; i++) {
                auto *jointNode = skin->joints[i];
                glm::mat4 jointMat = jointNode->get_matrix() * skin->inverse_bind_matrices[i];
                jointMat = inverseTransform * jointMat;
                mesh->uniformBlock.jointMatrix[i] = jointMat;
            }

            mesh->uniformBlock.jointcount = static_cast<float>(numJoints);

            mesh->ubo->update(&mesh->uniformBlock);
        } else {
            // TODO: FIX ME!
            // mesh->ubo->update(&m);
        }
    }

    for (auto &child : children) {
        child.update();
    }
}

} // namespace inexor::vulkan_renderer::gltf
