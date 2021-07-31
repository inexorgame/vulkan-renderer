#pragma once

#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::world {

class OctreeRenderer {
private:
    RenderGraph *m_render_graph;
    const TextureResource *m_back_buffer;
    const TextureResource *m_depth_buffer;
    wrapper::DescriptorBuilder &m_descriptor_builder;
    const std::vector<wrapper::Shader> &m_shaders;
    std::vector<wrapper::ResourceDescriptor> m_descriptors;
    std::vector<OctreeGpuVertex> m_octree_vertices;
    std::vector<std::uint32_t> m_octree_indices;
    BufferResource *m_octree_vertex_buffer{nullptr};
    BufferResource *m_octree_index_buffer{nullptr};

public:
    OctreeRenderer() = delete;

    /// @brief Default constructor.
    /// @param render_graph The rendergraph which is used
    /// @param back_buffer The back buffer which is used
    /// @param depth_buffer The depth buffer which is used
    /// @param shaders The shaders which are used
    /// @param descriptor_builder A const reference to a descriptor builder
    OctreeRenderer(RenderGraph *render_graph, const TextureResource *back_buffer, const TextureResource *depth_buffer,
                   const std::vector<wrapper::Shader> &shaders, wrapper::DescriptorBuilder &descriptor_builder);

    OctreeRenderer(const OctreeRenderer &) = delete;
    OctreeRenderer(OctreeRenderer &&) = delete;
    ~OctreeRenderer() = default;

    OctreeRenderer &operator=(const OctreeRenderer &) = delete;
    OctreeRenderer &operator=(OctreeRenderer &&) = delete;

    /// Render a give octree
    /// @param world The octree world to render
    /// @param uniform The octree's uniform buffer
    void render_octree(const world::Cube &world, wrapper::UniformBuffer uniform_buffer);
};

} // namespace inexor::vulkan_renderer::world
