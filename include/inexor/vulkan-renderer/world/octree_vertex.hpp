#pragma once

#include <glm/gtx/hash.hpp>
#include <glm/vec3.hpp>

namespace inexor::vulkan_renderer {

/// A struct for octree vertices
struct OctreeVertex {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 color{0.0f, 0.0f, 0.0f};
};

// Inline to suppress clang-tidy warning
inline bool operator==(const OctreeVertex &lhs, const OctreeVertex &rhs) {
    return lhs.position == rhs.position && lhs.color == rhs.color;
}

} // namespace inexor::vulkan_renderer

namespace std {

// Usually it is undefined behavior to declare something in the std namespace
// Specializing templates in the std namespace for user-defined types is an exception to the general rule of not
// modifying the std namespace
template <>
struct hash<inexor::vulkan_renderer::OctreeVertex> {
    std::size_t operator()(const inexor::vulkan_renderer::OctreeVertex &vertex) const {
        const auto h1 = std::hash<glm::vec3>{}(vertex.position);
        const auto h2 = std::hash<glm::vec3>{}(vertex.color);
        return h1 ^ h2;
    }
};

} // namespace std
