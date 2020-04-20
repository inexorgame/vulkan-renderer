#pragma once

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer {

/// TODO: Remove this!
struct InexorDimensions {
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);
};

} // namespace inexor::vulkan_renderer
