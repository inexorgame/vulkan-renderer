#pragma once

#include "inexor/vulkan-renderer/render-modules/octree/octree_vertex.hpp"

#include <glm/vec3.hpp>

#include <memory>

namespace inexor::vulkan_renderer::render_graph {
// Forward declarations
class Buffer;
class Texture;
class GraphicsPass;
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::tools {
// Forward declarations
class Camera;
} // namespace inexor::vulkan_renderer::tools

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Shader;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::descriptors {
// Forward declaration
class PerFrameDescriptorSets;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::wrapper::swapchains {
// Forward declaration
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper::swapchains

namespace inexor::vulkan_renderer::wrapper::pipelines {
// Forward declaration
class GraphicsPipeline;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_modules::octree {

// Using declaration
using render_graph::Buffer;
using render_graph::GraphicsPass;
using render_graph::RenderGraph;
using render_graph::Texture;
using vulkan_renderer::tools::Camera;
using wrapper::Shader;
using wrapper::descriptors::PerFrameDescriptorSets;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::swapchains::Swapchain;

/// @note We can exactly match the definition in the shader using data types in GLM.
/// The data in the matrices is binary compatible with the way the shader expects
/// it, so we can later just memcpy a UniformBufferObject to a VkBuffer.
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

/// A simple renderer for octree geometry
class OctreeRenderer {
private:
    // The vertex shader and fragment shader for octree rendering
    std::shared_ptr<Shader> m_vertex_shader;
    std::shared_ptr<Shader> m_fragment_shader;

    // The vertex buffer and index buffer for octree rendering
    std::weak_ptr<Buffer> m_vertex_buffer;
    std::weak_ptr<Buffer> m_index_buffer;

    // The matrix for model, view, and projection
    std::weak_ptr<Buffer> m_mvp_matrix;

    // The graphics pipeline for octree rendering
    std::shared_ptr<GraphicsPipeline> m_octree_pipeline;

    // The graphics pass for octree rendering
    std::weak_ptr<GraphicsPass> m_octree_pass;
    std::weak_ptr<PerFrameDescriptorSets> m_descriptor_set;

    std::weak_ptr<Camera> m_camera;

    std::weak_ptr<Swapchain> m_swapchain;
    std::weak_ptr<Texture> m_depth_buffer;

    std::vector<OctreeVertex> m_octree_vertices;
    std::vector<std::uint32_t> m_octree_indices;

    UniformBufferObject m_ubo;

    /// Flag to track if octree geometry has been updated and needs GPU buffer refresh
    bool m_geometry_updated{true}; // Initially true to upload geometry on first frame

public:
    /// Constructor
    /// @param render_graph The rendergraph to use for octree rendering
    /// @param swapchain The swapchain to use for octree rendering
    /// @param depth_buffer The depth buffer to use for octree rendering
    OctreeRenderer(std::shared_ptr<RenderGraph> render_graph, std::weak_ptr<Swapchain> swapchain,
                   std::weak_ptr<Texture> depth_buffer, std::shared_ptr<Camera> camera);

    void set_vertices_and_indices(std::vector<OctreeVertex> vertices, std::vector<std::uint32_t> indices);
};

} // namespace inexor::vulkan_renderer::render_modules::octree
