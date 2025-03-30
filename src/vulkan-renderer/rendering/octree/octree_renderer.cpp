#include "inexor/vulkan-renderer/rendering/octree/octree_renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"

namespace inexor::vulkan_renderer::rendering::octree {

using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::WriteDescriptorSetBuilder;

OctreeRenderer::OctreeRenderer(const std::weak_ptr<RenderGraph> rendergraph,
                               const std::weak_ptr<Texture> back_buffer,
                               const std::weak_ptr<Texture> depth_buffer) {
    if (rendergraph.expired()) {
        throw std::invalid_argument(
            "[OctreeRenderer::OctreeRenderer] Error: Parameter 'rendergraph' is not a valid pointer!");
    }
    if (back_buffer.expired()) {
        throw std::invalid_argument(
            "[OctreeRenderer::OctreeRenderer] Error: Parameter 'back_buffer' is not a valid pointer!");
    }
    if (depth_buffer.expired()) {
        throw std::invalid_argument(
            "[OctreeRenderer::OctreeRenderer] Error: Parameter 'depth_buffer' is not a valid pointer!");
    }
    // Get the rendergraph to work with here
    auto rg = rendergraph.lock();

    // Add a resource descriptor for the uniform buffer
    rg->add_resource_descriptor(
        [&](DescriptorSetLayoutBuilder &builder) {
            m_desc_set_layout = builder.add_uniform_buffer(VK_SHADER_STAGE_VERTEX_BIT).build("model/view/proj");
        },
        [&](DescriptorSetAllocator &allocator) {
            m_desc_set = allocator.allocate("model/view/proj", m_desc_set_layout);
        },
        [&](WriteDescriptorSetBuilder &builder) {
            return builder.add_uniform_buffer_update(m_desc_set, m_mvp_matrix_buffer).build();
        });

    m_octree_fragment =
        std::make_shared<Shader>(rg->device(), "Octree", VK_SHADER_STAGE_VERTEX_BIT, "shaders/octree.vert.spv");
    m_octree_vertex =
        std::make_shared<Shader>(rg->device(), "Octree", VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/octree.frag.spv");

    rg->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        m_octree_pipeline = builder.add_shader(m_octree_fragment)
                                .add_shader(m_octree_fragment)
                                .set_vertex_input_attributes(std::vector<VkVertexInputAttributeDescription>{
                                    {
                                        .location = 0,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(OctreeVertex, pos),
                                    },
                                    {
                                        .location = 1,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(OctreeVertex, color),
                                    },
                                })
                                .build("Octree");
    });

    m_octree_pass =
        rg->add_graphics_pass(rg->get_graphics_pass_builder()
                                  .set_on_record([&](const CommandBuffer &cmd_buf) {
                                      // TODO: We could move the pipeline and other stuff to Cube wrapper?
                                      for (const auto &root_cube : m_root_cubes) {
                                          cmd_buf
                                              .bind_pipeline(m_octree_pipeline)
                                              // TODO: Bind the root_cube's vertex buffer
                                              .bind_vertex_buffer(m_vertex_buffer)
                                              // TODO: Bind the root_cube's index buffer
                                              .bind_index_buffer(m_index_buffer)
                                              .bind_descriptor_set(m_desc_set, m_octree_pipeline)
                                              // TODO: Render the root cube's number of vertices
                                              .draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
                                      }
                                  })
                                  .writes_to(back_buffer)
                                  .writes_to(depth_buffer)
                                  .build("Octree", DebugLabelColor::RED));
}

void OctreeRenderer::add_root_cube(std::weak_ptr<Cube> root_cube) {
    // TODO: Create vertex and index buffer for the Cube here and store it in Cube wrapper?
    /*
        m_vertex_buffer = rg->add_buffer("Octree", BufferType::VERTEX_BUFFER,
                                     [&]() { m_vertex_buffer.lock()->request_update(m_octree_vertices); });

        m_index_buffer = rg->add_buffer("Octree", BufferType::INDEX_BUFFER,
                                    [&]() { m_index_buffer.lock()->request_update(m_octree_indices); });
    */
    // TODO: We could also store vertex and index buffersin OctreeRenderer here
    // std::vector<std::weak_ptr<Buffer>> m_vertex_buffers;
    // std::vector<std::weak_ptr<Buffer>> m_index_buffers;
    m_root_cubes.emplace_back(std::move(root_cube));
}

} // namespace inexor::vulkan_renderer::rendering::octree
