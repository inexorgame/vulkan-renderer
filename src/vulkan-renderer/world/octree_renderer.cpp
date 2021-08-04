#include "inexor/vulkan-renderer/world/octree_renderer.hpp"

#include <cassert>
#include <unordered_map>

namespace inexor::vulkan_renderer::world {

OctreeRenderer::OctreeRenderer(RenderGraph *render_graph, const TextureResource *back_buffer,
                               const TextureResource *depth_buffer, const std::vector<wrapper::Shader> &shaders)
    : m_render_graph(render_graph), m_back_buffer(back_buffer), m_depth_buffer(depth_buffer), m_shaders(shaders) {
    assert(render_graph);
    assert(back_buffer);
    assert(depth_buffer);
    assert(!shaders.empty());
}

void OctreeRenderer::render_octree(const world::Cube &world, const wrapper::UniformBuffer &uniform_buffer,
                                   wrapper::DescriptorBuilder &descriptor_builder) {
    m_octree_vertices.clear();

    for (const auto &polygons : world.polygons(true)) {
        for (const auto &triangle : *polygons) {
            for (const auto &vertex : triangle) {
                glm::vec3 color = {
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                };
                m_octree_vertices.emplace_back(vertex, color);
            }
        }
    }

    auto old_vertices = std::move(m_octree_vertices);

    m_octree_indices.clear();
    m_octree_vertices.clear();

    std::unordered_map<OctreeGpuVertex, std::uint32_t> vertex_map;

    for (auto &vertex : old_vertices) {
        // TODO: Use std::unordered_map::contains() when we switch to C++ 20.
        if (vertex_map.count(vertex) == 0) {
            assert(vertex_map.size() < std::numeric_limits<std::uint32_t>::max() && "Octree too big!");
            vertex_map.emplace(vertex, static_cast<std::uint32_t>(vertex_map.size()));
            m_octree_vertices.push_back(vertex);
        }

        m_octree_indices.push_back(vertex_map.at(vertex));
    }

    spdlog::trace("Reduced octree by {} vertices (from {} to {})", old_vertices.size() - m_octree_vertices.size(),
                  old_vertices.size(), m_octree_vertices.size());

    spdlog::trace("Total indices {} ", m_octree_indices.size());

    m_octree_index_buffer = m_render_graph->add<BufferResource>("octree index buffer", BufferUsage::INDEX_BUFFER);

    m_octree_index_buffer->upload_data(m_octree_indices);

    m_octree_vertex_buffer = m_render_graph->add<BufferResource>("octree vertex buffer", BufferUsage::VERTEX_BUFFER);

    m_octree_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT,
                                                 offsetof(OctreeGpuVertex, position)); // NOLINT

    m_octree_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT,
                                                 offsetof(OctreeGpuVertex, color)); // NOLINT

    m_octree_vertex_buffer->upload_data(m_octree_vertices);

    auto *octree_stage = m_render_graph->add<GraphicsStage>("octree stage");
    octree_stage->writes_to(m_back_buffer);
    octree_stage->writes_to(m_depth_buffer);
    octree_stage->reads_from(m_octree_index_buffer);
    octree_stage->reads_from(m_octree_vertex_buffer);
    octree_stage->bind_buffer(m_octree_vertex_buffer, 0);
    octree_stage->bind_buffer(m_octree_index_buffer, 0);
    octree_stage->set_clears_screen(true);

    // TODO: Remove this again as soon as glTF rendering works. We don't need that.
    octree_stage->set_depth_options(true, true);

    m_descriptors.push_back(descriptor_builder.add_uniform_buffer<UniformBufferObject>(uniform_buffer.buffer())
                                .build("octree uniform buffer"));

    octree_stage->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor(m_descriptors[0], physical.pipeline_layout());
        cmd_buf.draw_indexed(m_octree_indices.size());
    });

    for (const auto &shader : m_shaders) {
        octree_stage->uses_shader(shader);
    }

    octree_stage->add_descriptor_layout(m_descriptors[0].descriptor_set_layout());
}

} // namespace inexor::vulkan_renderer::world
