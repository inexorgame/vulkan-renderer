#include "inexor/vulkan-renderer/render-modules/octree/octree_renderer.hpp"

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/per_frame_descriptor_sets.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <fmt/color.h>
#include <spdlog/fmt/bundled/color.h>
#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::render_modules::octree {

OctreeRenderer::OctreeRenderer(std::shared_ptr<RenderGraph> render_graph, std::weak_ptr<Swapchain> swapchain,
                               std::weak_ptr<Texture> depth_buffer, std::shared_ptr<Camera> camera)
    : m_swapchain(std::move(swapchain)), m_depth_buffer(std::move(depth_buffer)), m_camera(std::move(camera)) {
    // Using declarations
    using render_graph::BufferType;
    using render_graph::GraphicsPassBuilder;
    using tools::InexorException;
    using tools::make_info;
    using wrapper::DebugLabelColor;
    using wrapper::pipelines::GraphicsPipelineBuilder;

    if (!render_graph) {
        throw InexorException("Error: Parameter 'render_graph' is invalid!");
    }
    if (m_swapchain.expired()) {
        throw InexorException("Error: Parameter 'swapchain' is invalid!");
    }
    if (m_depth_buffer.expired()) {
        throw InexorException("Error: Parameter 'depth_buffer' is invalid!");
    }
    if (m_camera.expired()) {
        throw InexorException("Error: Parameter 'camera' is invalid!");
    }

    // @TODO Maybe it is a good idea to let rendergraph load shaders and to abstract shader loading in it?

    // Load vertex and fragment shader for octree rendering
    // @TODO Use spirv-cross to load shaders and determine type automatically
    m_vertex_shader =
        std::make_shared<Shader>(render_graph->device(), VK_SHADER_STAGE_VERTEX_BIT, "shaders/main.vert.spv");
    m_fragment_shader =
        std::make_shared<Shader>(render_graph->device(), VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/main.frag.spv");

    m_mvp_matrix = render_graph->add_buffer("model/view/proj", BufferType::UNIFORM_BUFFER, [&]() {
        // @TODO Restrict external code from access to request_update method by implementing something like a
        // BufferUpdateBuilder, which is given to the lambda as a parameter. Maybe this can even help us to simplify the
        // pipeline barrier placement!
        m_ubo.model = glm::mat4(1.0f);
        m_ubo.view = m_camera.lock()->view_matrix();
        m_ubo.proj = m_camera.lock()->perspective_matrix();
        m_ubo.proj[1][1] *= -1;
        m_mvp_matrix.lock()->request_update(m_ubo);
    }, render_graph::BufferUpdateMode::PER_FRAME_HOST_VISIBLE);

    // Descriptor management for the model/view/projection uniform buffer
    m_descriptor_set = render_graph->add_resource_descriptor(m_mvp_matrix, VK_SHADER_STAGE_VERTEX_BIT);

    m_vertex_buffer = render_graph->add_buffer("vertex buffer", BufferType::VERTEX_BUFFER, [&]() {
        // Upload once after render-graph/swapchain recreation as well, because the new buffer starts null.
        const auto vertex_buffer = m_vertex_buffer.lock();
        const bool needs_initial_upload = vertex_buffer && vertex_buffer->buffer() == VK_NULL_HANDLE;
        if ((m_geometry_updated || needs_initial_upload) && !m_octree_vertices.empty()) {
            vertex_buffer->request_update(m_octree_vertices);
        }
    });

    m_index_buffer = render_graph->add_buffer("index buffer", BufferType::INDEX_BUFFER, [&]() {
        // Upload once after render-graph/swapchain recreation as well, because the new buffer starts null.
        const auto index_buffer = m_index_buffer.lock();
        const bool needs_initial_upload = index_buffer && index_buffer->buffer() == VK_NULL_HANDLE;
        if ((m_geometry_updated || needs_initial_upload) && !m_octree_indices.empty()) {
            index_buffer->request_update(m_octree_indices);
            // Reset after index update has been requested so vertex+index uploads stay in sync.
            m_geometry_updated = false;
        }
    });

    // Add the graphics pipeline for the octree renderer
    render_graph->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        const auto pipeline_extent = m_swapchain.lock()->extent();
        const auto descriptor_set = m_descriptor_set.lock();

        // The octree graphics pipeline is stored in the octree renderer
        // It is being build in this lambda by reference capture
        m_octree_pipeline = builder.add_shader(m_vertex_shader)
                                .add_shader(m_fragment_shader)
                                .set_vertex_input_bindings({{
                                    .binding = 0,
                                    .stride = sizeof(OctreeVertex),
                                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                                }})
                                .set_vertex_input_attributes({
                                    {
                                        .location = 0,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(OctreeVertex, position),
                                    },
                                    {
                                        .location = 1,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(OctreeVertex, color),
                                    },
                                })
                                .add_standard_alpha_blend_attachment()
                                .set_depth_attachment_format(m_depth_buffer.lock()->format())
                                .set_standard_depth_stencil()
                                .add_color_attachment_format(m_swapchain.lock()->image_format())
                                .set_dynamic_scissor()
                                .set_dynamic_viewport()
                                .set_viewport({
                                    .width = static_cast<float>(pipeline_extent.width),
                                    .height = static_cast<float>(pipeline_extent.height),
                                    .minDepth = 0.0f,
                                    .maxDepth = 1.0f,
                                })
                                .set_scissor({
                                    .extent = pipeline_extent,
                                })
                                .set_descriptor_set_layout(descriptor_set->layout())
                                .add_descriptor_set(m_descriptor_set)
                                .build("Octree");
    });

    // Add the graphics pass for the octree renderer
    m_octree_pass = render_graph->add_graphics_pass([&](GraphicsPassBuilder &builder) {
        return builder.writes_to(m_swapchain, VkClearValue{0.0f, 0.0f, 0.0f})
            .writes_to(m_depth_buffer, VkClearValue{.depthStencil = {.depth = 1.0f, .stencil = 0}})
            .reads_from(m_vertex_buffer)
            .writes_to(m_index_buffer)
            .reads_from(m_index_buffer)
            .set_on_record([&](const CommandBuffer &cmd_buf) {
                const auto vertex_buffer = m_vertex_buffer.lock();
                const auto index_buffer = m_index_buffer.lock();
                const auto swapchain = m_swapchain.lock();
                if (!vertex_buffer || !index_buffer || vertex_buffer->buffer() == VK_NULL_HANDLE ||
                    index_buffer->buffer() == VK_NULL_HANDLE || m_octree_indices.empty() || !swapchain) {
                    return;
                }
                const auto swapchain_extent = swapchain->extent();
                cmd_buf.bind_pipeline(m_octree_pipeline)
                    .bind_descriptor_set(m_descriptor_set)
                    .bind_vertex_buffer(m_vertex_buffer)
                    .bind_index_buffer(m_index_buffer)
                    .set_viewport({
                        .width = static_cast<float>(swapchain_extent.width),
                        .height = static_cast<float>(swapchain_extent.height),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f,
                    })
                    .set_scissor({
                        .extent = swapchain_extent,
                    })
                    .draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
            })
            .build("Octree", DebugLabelColor::GREEN);
    });
}

void OctreeRenderer::set_vertices_and_indices(std::vector<OctreeVertex> vertices, std::vector<std::uint32_t> indices) {
    if (m_octree_vertices == vertices && m_octree_indices == indices) {
        return;
    }

    m_octree_vertices = std::move(vertices);
    m_octree_indices = std::move(indices);
    m_geometry_updated = true;
}

} // namespace inexor::vulkan_renderer::render_modules::octree
