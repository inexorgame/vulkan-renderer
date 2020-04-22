#pragma once

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer::gltf_model {

///
struct BoundingBox {
    glm::vec3 min;

    glm::vec3 max;

    bool valid = false;

    BoundingBox() = default;

    BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}

    ///
    ///
    BoundingBox getAABB(glm::mat4 m) {
        glm::vec3 min = glm::vec3(m[3]);
        glm::vec3 max = min;
        glm::vec3 v0, v1;

        glm::vec3 right = glm::vec3(m[0]);
        v0 = right * this->min.x;
        v1 = right * this->max.x;
        min += glm::min(v0, v1);
        max += glm::max(v0, v1);

        glm::vec3 up = glm::vec3(m[1]);
        v0 = up * this->min.y;
        v1 = up * this->max.y;
        min += glm::min(v0, v1);
        max += glm::max(v0, v1);

        glm::vec3 back = glm::vec3(m[2]);
        v0 = back * this->min.z;
        v1 = back * this->max.z;
        min += glm::min(v0, v1);
        max += glm::max(v0, v1);

        return BoundingBox(min, max);
    }
};

} // namespace inexor::vulkan_renderer::gltf_model
