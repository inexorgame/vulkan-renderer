#include "inexor/vulkan-renderer/renderers/octree_renderer.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::render_components {

OctreeRenderer::OctreeRenderer(const wrapper::Device &device, render_graph::RenderGraph *render_graph)
    // Load the vertex shader and fragment shader for octree rendering
    : m_vertex_shader(m_device, VK_SHADER_STAGE_VERTEX_BIT, "Octree", "shaders/main.vert.spv"),
      m_fragment_shader(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "Octree", "shaders/main.frag.spv") {

    // Reserve memory for the vertex and index buffers
    m_vertex_buffers.reserve(m_worlds.size());
    m_index_buffers.reserve(m_worlds.size());

    spdlog::trace("Creating vertex and index buffer resources");

    // Create one vertex buffer and one index buffer per octree
    for (std::size_t octree_index = 0; octree_index < m_octrees; octree_index++) {
        m_vertex_buffer.emplace_back(render_graph::BufferUsage::VERTEX_BUFFER,
                                     std::string("Octree ") + std::to_string(octree_index), std::nullopt);
        m_index_buffer.emplace_back(render_graph::BufferUsage::INDEX_BUFFER,
                                    std::string("Octree ") + std::to_string(octree_index), std::nullopt)
    }

    spdlog::trace("Setting graphics stage for octree");

    const auto &stage_builder = render_graph->stage_builder();
    // Create the graphics stage for octree rendering
    render_graph->add_stage(
        // TODO: Implement graphics stage builder!
        stage_builder
            // TODO: .set_all_other_stuff
            // TODO: Descriptor management!
            // TODO: How to bind vertex and index buffers?
            .uses_shader(m_vertex_shader)
            .uses_shader(m_fragment_shader)
            .set_on_record([&](const render_graph::GraphicsStage &stage, const wrapper::CommandBuffer &cmd_buf) {
                // TODO: We will support multiple pipelines, right?
                // TODO: Do we even need the pipeline wrapper here?
                // TODO: How to pass world and perspective matrix into here?
                // TODO: How to pass back buffer in here?
                // TODO: How to pass depth buffer in here?
                // I think the OctreeRenderer should accept those two resources instead of abstracting/hiding it into
                // rendergraph!
                // TODO: render each octree! (bind vertex/index buffers, pipeline... draw)
            })
            // TODO: It is important not to use any objects whose lifetime would end with the constructor in here
            .set_on_update([&]() {
                // This update is being called before every frame
                // Loop through all octrees and check if we need to update vertices and indices
                for (std::size_t octree_index = 0; octree_index < m_octrees.size(); octree_index++) {
                    if (update_needed[octree_index]) {
                        // Update the vertices and indices of this octree
                        generate_octree_vertices(octree_index);
                        generate_octree_indices(octree_index);

                        // Copy the new data into the rendergraph
                        render_graph->update_buffer(m_vertex_buffers[octree_index],
                                                    m_vertex_buffers[octree_index].data() sizeof(world::OctreeVertex) *
                                                        m_vertex_buffers.size());
                        render_graph->update_buffer(m_index_buffers[octree_index] m_index_buffers[octree_index].data(),
                                                    sizeof(std::uint32_t) * m_index_buffers.size());

                        // Update is finished
                        update_needed[octree_index] = false;
                    }
                }
            })
            .build("Octree"));
}

void OctreeRenderer::regenerate_random_octree_geometry() {
    m_octrees.clear();
    // We currently create 2 random octrees
    // TODO: Abstract this into octree manager!
    m_octrees.push_back(
        world::create_random_world(2, {0.0f, 0.0f, 0.0f}, initialize ? std::optional(42) : std::nullopt));
    m_octrees.push_back(
        world::create_random_world(2, {10.0f, 0.0f, 0.0f}, initialize ? std::optional(60) : std::nullopt));
}

void OctreeRenderer::generate_octree_vertices(const std::size_t octree_index) {
    m_octree_vertices[octree_index].clear();
    for (const auto &polygons : m_octrees[octree_index]->polygons(true)) {
        for (const auto &triangle : *polygons) {
            for (const auto &vertex : triangle) {
                // TODO: Improve random color generation (use C++11 random tools)
                // TODO: Implement generate_random_color()?
                glm::vec3 color = {
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                };
                m_octree_vertices[octree_index].emplace_back(vertex, color);
            }
        }
    }
}

void OctreeRenderer::generate_octree_indices(const std::size_t octree_index) {
    auto old_vertices = std::move(m_octree_vertices[octree_index]);

    // Note: In case you are wondering if we are still allowed to use m_octree_vertices after the std::move:
    // The C++ standard does not define the specific state of an object after it has been moved from. The moved-from
    // object's state is unspecified, and it may be in a valid but unpredictable state depending on the implementation
    // We can bring it back into a specified state by calling clear() on it
    m_octree_vertices[octree_index].clear();
    m_octree_indices[octree_index].clear();

    std::unordered_map<world::OctreeVertex, std::uint32_t> vertex_map;
    for (auto &vertex : old_vertices) {
        if (vertex_map.contains(vertex)) {
            if (vertex_map.size() < std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error("Error: The octree is too big!");
            }
            vertex_map.emplace(vertex, static_cast<std::uint32_t>(vertex_map.size()));
            m_octree_vertices[octree_index].push_back(vertex);
        }
        m_octree_indices[octree_index].push_back(vertex_map.at(vertex));
    }
    spdlog::trace("Reduced octree by {} vertices (from {} to {})",
                  old_vertices.size() - m_octree_vertices[octree_index].size(), old_vertices.size(),
                  m_octree_vertices.size());
    spdlog::trace("Total indices {} ", m_octree_indices[octree_index].size());
}

void OctreeRenderer::regenerate_all_octree_vertices() {
    for (std::size_t octree_index = 0; octree_index < m_octrees.size(); octree_index++) {
        generate_octree_vertices(octree_index);
    }
}

void OctreeRenderer::regenerate_all_octree_indices() {
    for (std::size_t octree_index = 0; octree_index < m_octrees.size(); octree_index++) {
        generate_octree_indices(octree_index);
    }
}

} // namespace inexor::vulkan_renderer::render_components
