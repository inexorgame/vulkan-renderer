#pragma once

#include <glm/glm.hpp>

namespace inexor {
namespace vulkan_renderer {

/// TODO: Remove this!
struct InexorDimensions {
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);
};

}; // namespace vulkan_renderer
}; // namespace inexor
