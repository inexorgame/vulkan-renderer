#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace inexor::vulkan_renderer::gltf {

/// @brief A structure for bounding boxes
struct BoundingBox {
    glm::vec3 min{};
    glm::vec3 max{};
    bool valid{false};

    BoundingBox() = default;

    ///
    ///
    ///
    BoundingBox(const glm::vec3 min, const glm::vec3 max) : min(min), max(max){};

    ///
    ///
    ///
    [[nodiscard]] BoundingBox get_aabb(const glm::mat4 m) const {
        auto min = glm::vec3(m[3]);
        glm::vec3 max = min;
        glm::vec3 v0;
        glm::vec3 v1;

        auto right = glm::vec3(m[0]);
        v0 = right * this->min.x;
        v1 = right * this->max.x;
        min += glm::min(v0, v1);
        max += glm::max(v0, v1);

        auto up = glm::vec3(m[1]);
        v0 = up * this->min.y;
        v1 = up * this->max.y;
        min += glm::min(v0, v1);
        max += glm::max(v0, v1);

        auto back = glm::vec3(m[2]);
        v0 = back * this->min.z;
        v1 = back * this->max.z;
        min += glm::min(v0, v1);
        max += glm::max(v0, v1);

        return BoundingBox(min, max);
    }
};

} // namespace inexor::vulkan_renderer::gltf
