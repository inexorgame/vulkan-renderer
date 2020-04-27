#pragma once

#include <glm/glm.hpp>

#include <array>

namespace inexor::vulkan_renderer::gltf_model {

struct ModelVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv0;
    glm::vec2 uv1;
    glm::vec4 joint0;
    glm::vec4 weight0;

    static VkVertexInputBindingDescription get_vertex_binding_description();

    /// @note You should use the format where the amount of color channels matches the number of components in the shader data type.
    /// It is allowed to use more channels than the number of components in the shader, but they will be silently discarded.
    static std::array<VkVertexInputAttributeDescription, 6> get_attribute_binding_description();
};

} // namespace inexor::vulkan_renderer::gltf_model
