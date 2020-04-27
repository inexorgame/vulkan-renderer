#include "inexor/vulkan-renderer/gltf-model/vertex.hpp"

namespace inexor::vulkan_renderer::gltf_model {
VkVertexInputBindingDescription ModelVertex::get_vertex_binding_description() {
    VkVertexInputBindingDescription vertex_input_binding_description = {};

    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = sizeof(ModelVertex);
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return vertex_input_binding_description;
}

std::array<VkVertexInputAttributeDescription, 6> ModelVertex::get_attribute_binding_description() {
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
} // namespace inexor::vulkan_renderer::gltf_model
