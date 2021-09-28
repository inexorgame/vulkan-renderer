#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_primitive.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

/// @brief A struct for glTF2 model nodes.
/// @warning Since glTF2 models can be very huge, we could run into out of stack problems when calling the
/// destructors of the tree's nodes.
struct ModelNode {
    ModelNode *parent{nullptr};
    std::string name;
    std::uint32_t index;
    std::uint32_t skin_index;
    std::vector<ModelNode> children;
    std::vector<ModelPrimitive> mesh;
    glm::vec3 translation{};
    glm::vec3 scale{1.0f};
    glm::quat rotation{};
    glm::mat4 matrix{};
};

} // namespace inexor::vulkan_renderer::gltf
