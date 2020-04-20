#pragma once

#include <glm/glm.hpp>

namespace inexor::vulkan_renderer {

/// @class UniformBufferObject
/// @note We can exactly match the definition in the shader using data types in GLM.
/// The data in the matrices is binary compatible with the way the shader expects
/// it, so we can later just memcpy a UniformBufferObject to a VkBuffer.
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

} // namespace inexor::vulkan_renderer
