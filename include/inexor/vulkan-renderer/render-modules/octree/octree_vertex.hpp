#pragma once

#include <glm/gtx/hash.hpp>
#include <glm/vec3.hpp>

namespace inexor::vulkan_renderer::render_modules::octree {

struct OctreeVertex {
    glm::vec3 position;
    glm::vec3 color;

    OctreeVertex(glm::vec3 position, glm::vec3 color) : position(position), color(color) {}
};

// inline to suppress clang-tidy warning.
inline bool operator==(const OctreeVertex &lhs, const OctreeVertex &rhs) {
    return lhs.position == rhs.position && lhs.color == rhs.color;
}

} // namespace inexor::vulkan_renderer::render_modules::octree

namespace std {

template <>
struct hash<inexor::vulkan_renderer::render_modules::octree::OctreeVertex> {
    std::size_t operator()(const inexor::vulkan_renderer::render_modules::octree::OctreeVertex &vertex) const {
        const auto h1 = std::hash<glm::vec3>{}(vertex.position);
        const auto h2 = std::hash<glm::vec3>{}(vertex.color);
        return h1 ^ h2;
    }
};

} // namespace std
