#pragma once

#include <glm/gtx/hash.hpp>
#include <glm/vec3.hpp>

namespace inexor::vulkan_renderer {

struct OctreeGpuVertex {
    glm::vec3 position;
    glm::vec3 color;

    OctreeGpuVertex(glm::vec3 position, glm::vec3 color) : position(position), color(color) {}
};

// inline to suppress clang-tidy warning.
inline bool operator==(const OctreeGpuVertex &lhs, const OctreeGpuVertex &rhs) {
    return lhs.position == rhs.position && lhs.color == rhs.color;
}

} // namespace inexor::vulkan_renderer

namespace std {

template <>
struct hash<inexor::vulkan_renderer::OctreeGpuVertex> {
    std::size_t operator()(const inexor::vulkan_renderer::OctreeGpuVertex &vertex) const {
        const auto h1 = std::hash<glm::vec3>{}(vertex.position);
        const auto h2 = std::hash<glm::vec3>{}(vertex.color);
        return h1 ^ h2;
    }
};

} // namespace std
