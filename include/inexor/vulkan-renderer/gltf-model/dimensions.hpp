#pragma once

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer::gltf_model {

/// TODO: Remove this!
struct Dimensions {
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);
};

} // namespace inexor::vulkan_renderer::gltf_model
