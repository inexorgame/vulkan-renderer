#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_animation.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_mesh.hpp"
#include "inexor/vulkan-renderer/gltf/gltf_primitive.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

// forward declaration
struct ModelNode;

// TODO: This should be in the animation file, but circular deps when including make this tricky
struct ModelSkin {
    std::string name;
    ModelNode *skeleton_root{nullptr};
    std::vector<glm::mat4> inverse_bind_matrices;
    std::vector<ModelNode *> joints;
};

/// @brief A struct for glTF2 model nodes.
/// @warning Since glTF2 models can be very huge, we could run into out of stack problems when calling the
/// destructors of the tree's nodes.
struct ModelNode {
    ModelNode *parent{nullptr};
    std::string name;
    std::uint32_t index;
    ModelSkin *skin{nullptr};
    std::uint32_t skin_index;
    std::vector<ModelNode> children;
    std::unique_ptr<ModelMesh> mesh;
    glm::vec3 translation{};
    glm::vec3 scale{1.0f};
    glm::quat rotation{};
    glm::mat4 matrix{};
    BoundingBox bvh;
    BoundingBox aabb;
    glm::mat4 local_matrix();
    glm::mat4 get_matrix();
    void update();
};

} // namespace inexor::vulkan_renderer::gltf
