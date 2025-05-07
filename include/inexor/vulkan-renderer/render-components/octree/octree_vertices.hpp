#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <glm/vec3.hpp>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::descriptors {
class DescriptorSetLayout;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::wrapper::pipelines {
class GraphicsPipeline;
class PipelineLayout;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::wrapper {
class Shader;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::rendering::octree {

// Using-declarations
using wrapper::Shader;
using wrapper::descriptors::DescriptorSetLayout;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::pipelines::PipelineLayout;

// NOTE: It makes no sense to keep the vertex or index buffer of a cube in the Cube class:
// 1) The Cube class should only be the data structure, and it should be decouples from rendering!
// 2) Each material comes with its own vertex structure (VkVertexInputAttributeDescription,
// VkVertexInputBindingDescription), which means for every material we will likely need a separate vertex buffer!

/// This is the data which every material needs to have
/// Every material is unique, but can have an arbitrary number of instances
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

// This is the data which every material instance needs to have
struct MaterialInstanceBase {
    std::weak_ptr<Buffer> vertex_buffer;
    std::weak_ptr<Buffer> index_buffer;
    // TODO: Descriptor sets here?
};

/// This is the data every octree is required to have to be rendered
struct OctreeMaterialInstanceBase : public MaterialInstanceBase {
    glm::vec3 position;
    glm::float32 scale;
    glm::quat rotation;
    glm::mat4 model;
};

/// The struct used in the shader
struct SimpleColoredVertex {
    glm::vec3 position;
    glm::vec3 color;
};

// This is a very simple octree material
struct SimpleColoredTrianglesOctreeMaterial : public OctreeMaterialInstanceBase {
    // This is the data required to generate the buffers
    std::vector<SimpleColoredVertex> vertices;
    std::vector<std::uint32_t> indices;
};

} // namespace inexor::vulkan_renderer::rendering::octree
