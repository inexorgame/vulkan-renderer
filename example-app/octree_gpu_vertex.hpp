#pragma once

#include <glm/gtx/hash.hpp>
#include <glm/vec3.hpp>

namespace inexor::example_app {

struct OctreeGpuVertex {
    glm::vec3 position;
    glm::vec3 color;

    OctreeGpuVertex(glm::vec3 position, glm::vec3 color) : position(position), color(color) {}
};

// inline to suppress clang-tidy warning.
inline bool operator==(const OctreeGpuVertex &lhs, const OctreeGpuVertex &rhs) {
    return lhs.position == rhs.position && lhs.color == rhs.color;
}

} // namespace inexor::example_app

namespace std {

template <>
struct hash<inexor::example_app::OctreeGpuVertex> {
    std::size_t operator()(const inexor::example_app::OctreeGpuVertex &vertex) const {
        const auto h1 = std::hash<glm::vec3>{}(vertex.position);
        const auto h2 = std::hash<glm::vec3>{}(vertex.color);
        return h1 ^ h2;
    }
};

} // namespace std
