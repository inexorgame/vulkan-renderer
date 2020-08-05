#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace inexor::vulkan_renderer {

struct OctreeGpuVertex {
    glm::vec3 position;
    glm::vec3 color;

    OctreeGpuVertex(glm::vec3 position, glm::vec3 color) : position(position), color(color) {}
};

// TODO: This shouldn't really be here
struct UiGpuVertex {
    glm::vec3 position;
    glm::vec2 tex_coords;

    UiGpuVertex(glm::vec3 position, glm::vec2 tex_coords) : position(position), tex_coords(tex_coords) {}
};

} // namespace inexor::vulkan_renderer
