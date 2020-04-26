#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <array>

namespace inexor::vulkan_renderer {

// TODO: Generalize this setup using a builder pattern!
struct OctreeVertex {
    glm::vec3 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription get_vertex_binding_description() {
        VkVertexInputBindingDescription vertex_input_binding_description = {};

        vertex_input_binding_description.binding = 0;
        vertex_input_binding_description.stride = sizeof(OctreeVertex);
        vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return vertex_input_binding_description;
    }

    /// @note You should use the format where the amount of color channels matches the number of components in the shader data type.
    /// It is allowed to use more channels than the number of components in the shader, but they will be silently discarded.
    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_binding_description() {
        std::array<VkVertexInputAttributeDescription, 2> vertex_input_attribute_description = {};

        vertex_input_attribute_description[0].location = 0;
        vertex_input_attribute_description[0].binding = 0;
        vertex_input_attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertex_input_attribute_description[0].offset = offsetof(OctreeVertex, pos);

        vertex_input_attribute_description[1].location = 1;
        vertex_input_attribute_description[1].binding = 0;
        vertex_input_attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertex_input_attribute_description[1].offset = offsetof(OctreeVertex, color);

        return vertex_input_attribute_description;
    }
};

} // namespace inexor::vulkan_renderer
