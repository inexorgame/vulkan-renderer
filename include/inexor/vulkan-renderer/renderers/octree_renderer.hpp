#pragma once

#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/world/octree_vertex.hpp"

#include <glm/mat4x4.hpp>

#include <cstdint>
#include <memory>
#include <vector>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_components {

/// Matrices for model, view, and projection
struct ModelViewProjectionMatrices {
    glm::mat4 model{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 proj{1.0f};
};

/// A class for rendering octree geometry
class OctreeRenderer {
private:
    /// The octrees to render
    std::vector<std::shared_ptr<world::Cube>> m_octrees;
    std::vector<bool> update_needed;

    /// The shaders for octree rendering
    wrapper::Shader m_vertex_shader;
    wrapper::shader m_fragment_shader;

    // There is one vector of vertices and indices for each octree
    std::vector<std::vector<world::OctreeVertex>> m_octree_vertices;
    std::vector<std::vector<std::uint32_t>> m_octree_indices;

    // There is one vertex buffer and one index buffer for each octree
    std::vector<render_graph::BufferResource> m_vertex_buffers;
    std::vector<render_graph::IndexBuffer> m_index_buffers;

    void generate_octree_vertices(std::size_t octree_index);
    void generate_octree_indices(std::size_t octree_index);

    void regenerate_all_octree_vertices();
    void regenerate_all_octree_indices();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param render_graph The render graph
    explicit OctreeRenderer(const wrapper::Device &device, render_graph::RenderGraph *render_graph);
    OctreeRenderer(const OctreeRenderer &) = delete;
    OctreeRenderer(OctreeRenderer &&) = delete;
    ~OctreeRenderer() = default;

    OctreeRenderer &operator=(const OctreeRenderer &) = delete;
    OctreeRenderer &operator=(OctreeRenderer &&) = delete;

    /// Creates random octree geometry
    void generate_random_octree_geometry();
};

} // namespace inexor::vulkan_renderer::render_components
