#pragma once

#include <glm/vec4.hpp>

namespace inexor::vulkan_renderer::gltf {

/// @brief A struct for glTF2 model materials.
struct ModelMaterial {
    glm::vec4 base_color_factor{1.0f};
    std::uint32_t base_color_texture_index{0};
};

} // namespace inexor::vulkan_renderer::gltf
