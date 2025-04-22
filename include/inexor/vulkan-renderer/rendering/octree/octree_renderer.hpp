#pragma once

#include "inexor/vulkan-renderer/rendering/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/rendering/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/rendering/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/rendering/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <memory>

namespace inexor::vulkan_renderer::rendering::octree {

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
using wrapper::commands::CommandBuffer;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::pipelines::GraphicsPipelineBuilder;
using wrapper::pipelines::PipelineLayout;

/// A data structure for octree vertices
struct OctreeVertex {
    glm::vec3 pos;
    glm::vec3 color;
};

/// The model, view, and projection matrix used in a uniform buffer for rendering
struct ModelViewProjMatrix {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

/// A rendering class for octrees
class OctreeRenderer {
private:
    std::shared_ptr<Shader> m_octree_vertex;
    std::shared_ptr<Shader> m_octree_fragment;
    std::weak_ptr<GraphicsPass> m_octree_pass;
    std::shared_ptr<GraphicsPipeline> m_octree_pipeline;

    std::weak_ptr<Buffer> m_mvp_matrix;

    /// The camera used for rendering
    std::weak_ptr<Camera> m_camera;

    VkDescriptorSetLayout m_desc_set_layout{VK_NULL_HANDLE};
    VkDescriptorSet m_desc_set{VK_NULL_HANDLE};

    /// The cubes we use for rendering
    std::vector<std::weak_ptr<Cube>> m_cubes;

public:
    /// Default constructor
    /// @param rendergraph The rendergraph used to build octree renderer on
    /// @param back_buffer The back buffer to render to
    /// @param depth_buffer The depth buffer to render to
    OctreeRenderer(std::weak_ptr<RenderGraph> rendergraph,
                   std::weak_ptr<Texture> back_buffer,
                   std::weak_ptr<Texture> depth_buffer);

    OctreeRenderer(const OctreeRenderer &) = delete;
    OctreeRenderer(OctreeRenderer &&) = delete;
    ~OctreeRenderer() = default;

    OctreeRenderer &operator=(const OctreeRenderer &) = delete;
    OctreeRenderer &operator=(OctreeRenderer &&) = delete;

    /// Add a new cube to the octree renderer
    /// @param cube The root cube to add to the renderer
    void add_octree(std::weak_ptr<Cube> cube);

    // Return the octree graphics pass
    [[nodiscard]] std::weak_ptr<GraphicsPass> get_pass() const {
        return m_octree_pass;
    }

    /// Remove a cube from the octree renderer
    /// @param cube The cube to remove
    void remove_octree(std::weak_ptr<Cube> cube);

    /// Set the camera used as view matrix for octree rendering
    /// @param camera The new camera to use for rendering
    void set_camera(std::weak_ptr<Camera> camera);
};

} // namespace inexor::vulkan_renderer::rendering::octree
