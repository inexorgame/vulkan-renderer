#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <array>

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
    // TODO: use std::array here and use offsetof() for vertex attribute layout?
    glm::vec4 joint{};
    glm::vec4 weight{};
};
} // namespace inexor::vulkan_renderer::gltf
