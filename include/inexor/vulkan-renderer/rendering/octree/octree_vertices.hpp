#pragma once

#include "inexor/vulkan-renderer/rendering/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"

#include <glm/vec3.hpp>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::rendering::octree {

// Notes:
// It makes no sense to keep the vertex or index buffer of a cube in the Cube class:
// 1) The Cube class should only be the data structure, and it should be decouples from rendering!
// 2) Each material comes with its own vertex structure (VkVertexInputAttributeDescription,
// VkVertexInputBindingDescription), which means for every material we will likely need a separate vertex buffer!

struct MaterialBase {
    std::string name;
    std::shared_ptr<Shader> vertex_shader;
    std::shared_ptr<Shader> fragment_shader;
    std::shared_ptr<PipelineLayout> pipeline_layout;
    std::shared_ptr<GraphicsPipeline> pipeline;
    VkPushConstantRange push_constant;
    std::shared_ptr<DescriptorSetLayout> descriptor_set_layout;
    std::vector<VkVertexInputBindingDescription> vertex_input_bindings;
    std::vector<VkVertexInputAttributeDescription> vertex_input_attributes;
};

// This data applies to the entire octree, and not individual vertices
struct OctreeMaterial {
    glm::vec3 position;
    glm::float32 scale;
    glm::quat rotation;
    glm::mat4 model;
    std::weak_ptr<Buffer> vertex_buffer;
    std::weak_ptr<Buffer> index_buffer;
};

struct OctreeMaterialInstance : public OctreeMaterial {};

/// A data structure for colored octree vertices
struct SimpleColoredVertex {
    glm::vec3 position;
    glm::vec3 color;
};

/// A simple material for colored triangles (3 colors per triangle, one for each vertex)
struct SimpleColoredTrianglesMaterial : public OctreeMaterial {
    // The vertices
    std::vector<SimpleColoredVertex> vertices;
    // TODO: Index buffer?
};

// TODO: Who creates the Buffers then?

} // namespace inexor::vulkan_renderer::rendering::octree
