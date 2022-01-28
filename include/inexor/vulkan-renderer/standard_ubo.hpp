#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace inexor::vulkan_renderer {

// TODO: Rename file?
struct DefaultUBO {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    glm::vec3 camera_pos;
};

struct SkyboxUBO {
    glm::mat4 projection;
    glm::mat4 model;
};

} // namespace inexor::vulkan_renderer
