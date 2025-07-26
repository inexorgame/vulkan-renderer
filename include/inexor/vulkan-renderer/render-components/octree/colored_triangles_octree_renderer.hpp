#pragma once

#include "inexor/vulkan-renderer/render-components/octree/colored_triangles_octree_material.hpp"
#include "inexor/vulkan-renderer/render-components/renderer_base.hpp"
#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <glm/glm.hpp>

#include <memory>

namespace inexor::vulkan_renderer::render_components::octree {

// Using declarations
using render_graph::Buffer;
using render_graph::BufferType;
using render_graph::RenderGraph;
using render_graph::Texture;
using vulkan_renderer::tools::Camera;
using world::Cube;
using wrapper::DebugLabelColor;
using wrapper::GraphicsPass;
using wrapper::Shader;
using wrapper::Swapchain;
using wrapper::commands::CommandBuffer;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::pipelines::GraphicsPipelineBuilder;
using wrapper::pipelines::PipelineLayout;

/// The model, view, and projection matrix used in a uniform buffer for rendering
struct ModelViewProjMatrix {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

/// Renderer for colored octrees (3 colors per triangle)
class ColoredTrianglesOctreeRenderer : public RendererBase {
private:
    VkFormat m_back_buffer_img_format;
    VkExtent2D m_back_buffer_extent;
    ModelViewProjMatrix m_mvp_data{};
    std::weak_ptr<Buffer> m_mvp_matrix;
    std::weak_ptr<Camera> m_camera;
    std::shared_ptr<Shader> m_vertex_shader;
    std::shared_ptr<Shader> m_fragment_shader;
    std::shared_ptr<GraphicsPipeline> m_graphics_pipeline;
    VkDescriptorSetLayout m_descriptor_set_layout;
    VkDescriptorSet m_descriptor_set;

    /// The instances of colored triangles octree material to render
    std::vector<std::weak_ptr<ColoredTrianglesOctreeMaterialInstance>> m_cubes;

public:
    /// Default constructor
    /// @param rendergraph The rendergraph used to build octree renderer on
    /// @param back_buffer The back buffer to render to
    /// @param depth_buffer The depth buffer to render to
    ColoredTrianglesOctreeRenderer(std::weak_ptr<RenderGraph> rendergraph,
                                   std::weak_ptr<Texture> back_buffer,
                                   std::weak_ptr<Texture> depth_buffer);

    /// Add a new cube to the octree renderer
    /// @param cube The root cube to add to the renderer
    void add_octree(std::weak_ptr<ColoredTrianglesOctreeMaterialInstance> cube);

    /// Remove a cube from the octree renderer
    /// @param cube The cube to remove
    void remove_octree(std::weak_ptr<ColoredTrianglesOctreeMaterialInstance> cube);

    /// Set the camera used as view matrix for octree rendering
    /// @param camera The new camera to use for rendering
    void set_camera(std::weak_ptr<Camera> camera);
};

} // namespace inexor::vulkan_renderer::render_components::octree
