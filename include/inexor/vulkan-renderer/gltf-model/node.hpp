#pragma once

#include "inexor/vulkan-renderer/gltf-model/bounding_box.hpp"
#include "inexor/vulkan-renderer/gltf-model/mesh.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::gltf_model {

struct ModelNode;

struct ModelSkin {
    std::string name;

    std::shared_ptr<ModelNode> skeleton_root = nullptr;

    std::vector<glm::mat4> inverse_bind_matrices;

    std::vector<std::shared_ptr<ModelNode>> joints;
};

struct ModelNode {
    std::shared_ptr<ModelNode> parent;

    uint32_t index;

    std::vector<std::shared_ptr<ModelNode>> children;

    glm::mat4 matrix;

    std::string name;

    std::shared_ptr<Mesh> mesh;

    std::shared_ptr<ModelSkin> skin;

    int32_t skin_index = -1;

    glm::vec3 translation{};

    glm::vec3 scale{1.0f};

    glm::quat rotation{};

    BoundingBox bvh;

    BoundingBox aabb;

    glm::mat4 local_matrix();

    glm::mat4 get_matrix();

    void update(const std::shared_ptr<UniformBufferManager> uniform_buffer_manager);
};

} // namespace inexor::vulkan_renderer::gltf_model
