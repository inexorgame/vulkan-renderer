#pragma once

#include "inexor/vulkan-renderer/gltf/animation.hpp"
#include "inexor/vulkan-renderer/gltf/mesh.hpp"
#include "inexor/vulkan-renderer/gltf/primitive.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

// Forward declaration
struct ModelNode;

// TODO: This should be in the animation file, but circular deps when including make this tricky
struct ModelSkin {
    std::string name;
    ModelNode *skeleton_root{nullptr};
    std::vector<glm::mat4> inverse_bind_matrices;
    std::vector<ModelNode *> joints;
};

/// A struct for glTF2 model nodes
struct ModelNode {
    ModelNode *parent{nullptr};
    std::string name;
    std::uint32_t index;
    ModelSkin *skin{nullptr};
    std::uint32_t skin_index;
    // TODO: Only take a vector of raw pointers?
    std::vector<std::shared_ptr<ModelNode>> children;
    std::shared_ptr<ModelMesh> mesh{nullptr};
    bool visible{true};
    glm::vec3 translation{};
    glm::vec3 scale{1.0f};
    glm::quat rotation{};
    glm::mat4 matrix{};
    BoundingBox bvh;
    BoundingBox aabb;

    [[nodiscard]] glm::mat4 local_matrix() const;
    [[nodiscard]] glm::mat4 get_matrix() const;

    void update();
};

} // namespace inexor::vulkan_renderer::gltf
