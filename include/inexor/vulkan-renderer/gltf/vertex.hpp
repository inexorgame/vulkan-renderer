#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "inexor/vulkan-renderer/vk_tools/vert_attr_layout.hpp"

#include <array>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

/// @brief A struct for glTF2 model vertices.
struct ModelVertex {
    ModelVertex() = default;

    /// @brief Overloaded constructor.
    /// @param position The position of the model vertex
    /// @param color_rgb The color of the model vertex
    ModelVertex(const glm::vec3 position, const glm::vec3 color_rgb) : pos(position), color(color_rgb) {}

    glm::vec3 pos{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec2 uv0{};
    glm::vec2 uv1{};
    glm::vec4 joint{};
    glm::vec4 weight{};

    static auto vertex_attribute_layout() {
        return std::vector<vk_tools::VertexAttributeLayout>{
            {VK_FORMAT_R32G32B32_SFLOAT, sizeof(pos), offsetof(ModelVertex, pos)},
            {VK_FORMAT_R32G32B32_SFLOAT, sizeof(color), offsetof(ModelVertex, color)},
            {VK_FORMAT_R32G32_SFLOAT, sizeof(normal), offsetof(ModelVertex, normal)},
            {VK_FORMAT_R32G32_SFLOAT, sizeof(uv1), offsetof(ModelVertex, uv1)},
            {VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(joint), offsetof(ModelVertex, joint)},
            {VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(weight), offsetof(ModelVertex, weight)}};
    }
};
} // namespace inexor::vulkan_renderer::gltf
