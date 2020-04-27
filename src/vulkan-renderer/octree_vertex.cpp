#include "inexor/vulkan-renderer/octree_vertex.hpp"

namespace inexor::vulkan_renderer {
VkVertexInputBindingDescription OctreeVertex::get_vertex_binding_description() {
    VkVertexInputBindingDescription vertex_input_binding_description = {};

    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = sizeof(OctreeVertex);
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return vertex_input_binding_description;
}

std::array<VkVertexInputAttributeDescription, 2> OctreeVertex::get_attribute_binding_description() {
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
} // namespace inexor::vulkan_renderer
