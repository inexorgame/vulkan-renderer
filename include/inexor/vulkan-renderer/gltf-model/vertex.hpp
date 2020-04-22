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

    static VkVertexInputBindingDescription get_vertex_binding_description() {
        VkVertexInputBindingDescription vertex_input_binding_description = {};

        vertex_input_binding_description.binding = 0;
        vertex_input_binding_description.stride = sizeof(ModelVertex);
        vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return vertex_input_binding_description;
    }

    /// @note You should use the format where the amount of color channels matches the number of components in the shader data type.
    /// It is allowed to use more channels than the number of components in the shader, but they will be silently discarded.
    static std::array<VkVertexInputAttributeDescription, 6> get_attribute_binding_description() {
        // TODO: Generalize this setup!
        std::array<VkVertexInputAttributeDescription, 6> vertex_input_attribute_description = {};

        vertex_input_attribute_description[0].location = 0;
        vertex_input_attribute_description[0].binding = 0;
        vertex_input_attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertex_input_attribute_description[0].offset = offsetof(ModelVertex, pos);

        vertex_input_attribute_description[1].location = 1;
        vertex_input_attribute_description[1].binding = 0;
        vertex_input_attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertex_input_attribute_description[1].offset = offsetof(ModelVertex, normal);

        vertex_input_attribute_description[2].location = 2;
        vertex_input_attribute_description[2].binding = 0;
        vertex_input_attribute_description[2].format = VK_FORMAT_R32G32_SFLOAT;
        vertex_input_attribute_description[2].offset = offsetof(ModelVertex, uv0);

        vertex_input_attribute_description[3].location = 3;
        vertex_input_attribute_description[3].binding = 0;
        vertex_input_attribute_description[3].format = VK_FORMAT_R32G32_SFLOAT;
        vertex_input_attribute_description[3].offset = offsetof(ModelVertex, uv1);

        vertex_input_attribute_description[4].location = 4;
        vertex_input_attribute_description[4].binding = 0;
        vertex_input_attribute_description[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        vertex_input_attribute_description[4].offset = offsetof(ModelVertex, joint0);

        vertex_input_attribute_description[5].location = 5;
        vertex_input_attribute_description[5].binding = 0;
        vertex_input_attribute_description[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        vertex_input_attribute_description[5].offset = offsetof(ModelVertex, weight0);

        return vertex_input_attribute_description;
    }
};

} // namespace inexor::vulkan_renderer::gltf_model
