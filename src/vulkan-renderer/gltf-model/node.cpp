#include "inexor/vulkan-renderer/gltf-model/node.hpp"

namespace inexor::vulkan_renderer::gltf_model {
glm::mat4 ModelNode::local_matrix() {
    return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
}

glm::mat4 ModelNode::get_matrix() {
    glm::mat4 m = local_matrix();

    std::shared_ptr<ModelNode> p = parent;

    while (p) {
        m = p->local_matrix() * m;
        p = p->parent;
    }

    return m;
}

void ModelNode::update(std::shared_ptr<UniformBufferManager> uniform_buffer_manager) {
    if (mesh) {
        glm::mat4 m = get_matrix();

        if (skin) {
            mesh->uniform_block.matrix = m;

            // Update join matrices.
            glm::mat4 inverseTransform = glm::inverse(m);

            std::size_t numJoints = std::min((std::uint32_t)skin->joints.size(), MAX_NUM_JOINTS);

            for (std::size_t i = 0; i < numJoints; i++) {
                std::shared_ptr<ModelNode> jointNode = skin->joints[i];

                glm::mat4 jointMat = jointNode->get_matrix() * skin->inverse_bind_matrices[i];
                jointMat = inverseTransform * jointMat;
                mesh->uniform_block.joint_matrix[i] = jointMat;
            }

            mesh->uniform_block.joint_count = (float)numJoints;

            spdlog::debug("Updating uniform buffers.");

            // Update uniform buffer.
            uniform_buffer_manager->update_uniform_buffer(mesh->uniform_buffer, &mesh->uniform_block, sizeof(glm::mat4));
        } else {
            spdlog::debug("Updating uniform buffers.");

            // Updat uniform buffer.
            uniform_buffer_manager->update_uniform_buffer(mesh->uniform_buffer, &m, sizeof(glm::mat4));
        }
    }

    for (auto &child : children) {
        child->update(uniform_buffer_manager);
    }
}
} // namespace inexor::vulkan_renderer::gltf_model
