#pragma once

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer::gltf_model {

struct BoundingBox {
    glm::vec3 min;

    glm::vec3 max;

    bool valid = false;

    BoundingBox() = default;
    BoundingBox(glm::vec3 min, glm::vec3 max);

    BoundingBox getAABB(glm::mat4 m);
};

} // namespace inexor::vulkan_renderer::gltf_model
