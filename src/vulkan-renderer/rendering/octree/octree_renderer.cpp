#include "inexor/vulkan-renderer/rendering/octree/octree_renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/exception.hpp"

#include <utility>

namespace inexor::vulkan_renderer::rendering::octree {

// Using declarations
using wrapper::InexorException;
using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::WriteDescriptorSetBuilder;

OctreeRenderer::OctreeRenderer(const std::weak_ptr<RenderGraph> rendergraph,
                               const std::weak_ptr<Texture> back_buffer,
                               const std::weak_ptr<Texture> depth_buffer) {
    if (rendergraph.expired()) {
        throw InexorException("Error: Parameter 'rendergraph' is an invalid pointer!");
    }
    if (back_buffer.expired()) {
        throw InexorException("Error: Parameter 'back_buffer' is an invalid pointer!");
    }
    if (depth_buffer.expired()) {
        throw InexorException("Error: Parameter 'depth_buffer' is an invalid pointer!");
    }

    auto rg = rendergraph.lock();
    if (rg == nullptr) {
        throw InexorException("Error: Parameter 'rendergraph' is an invalid pointer!");
    }

    // Descriptor management
    rg->add_resource_descriptor(
        [&](DescriptorSetLayoutBuilder &builder) {
            m_descriptor_set_layout = builder.add_uniform_buffer(VK_SHADER_STAGE_VERTEX_BIT).build("model/view/proj");
        },
        [&](DescriptorSetAllocator &allocator) {
            m_descriptor_set = allocator.allocate("model/view/proj", m_descriptor_set_layout);
        },
        [&](WriteDescriptorSetBuilder &builder) {
            return builder.add_uniform_buffer_update(m_descriptor_set, m_mvp_matrix).build();
        });

    // Load the shaders used for octree rendering
    m_octree_fragment =
        std::make_shared<Shader>(rg->device(), "Octree", VK_SHADER_STAGE_VERTEX_BIT, "shaders/octree.vert.spv");
    m_octree_vertex =
        std::make_shared<Shader>(rg->device(), "Octree", VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/octree.frag.spv");

    m_back_buffer_img_format = back_buffer.lock()->format();
    m_back_buffer_extent = back_buffer.lock()->extent();

    // Set the callback for creation of the graphics pipeline
    rg->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        m_octree_pipeline = builder.add_shader(m_octree_vertex)
                                .add_shader(m_octree_fragment)
                                .set_vertex_input_bindings({{
                                    .binding = 0,
                                    .stride = sizeof(OctreeVertex),
                                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                                }})
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
                                .set_scissor(m_back_buffer_extent)
                                .set_viewport(m_back_buffer_extent)
                                .add_default_color_blend_attachment()
                                .set_multisampling(VK_SAMPLE_COUNT_1_BIT, std::nullopt)
                                .add_color_attachment_format(m_back_buffer_img_format)
                                .set_descriptor_set_layout(m_descriptor_set_layout)
                                .build("Octree");
    });

    // Add the graphics pass for octree rendering
    m_octree_pass = rg->add_graphics_pass(rg->get_graphics_pass_builder()
                                              .set_on_record([&](const CommandBuffer &cmd_buf) {
                                                  // Render all the cubes passed to the octree renderer
                                                  for (const auto &cube : m_cubes) {
                                                      auto cb = cube.lock();
                                                      if (cb != nullptr) {
                                                          cmd_buf.bind_pipeline(m_octree_pipeline)
                                                              .bind_vertex_buffer(cb->m_vertex_buffer)
                                                              .bind_index_buffer(cb->m_index_buffer)
                                                              .bind_descriptor_set(m_descriptor_set, m_octree_pipeline)
                                                              .draw_indexed(cb->m_index_count);
                                                      }
                                                      // Note: We deliberately don't do error handling if the cube is
                                                      // nullptr because this is performance critical code, so there is
                                                      // no else block here
                                                  }
                                              })
                                              .writes_to(back_buffer)
                                              .writes_to(depth_buffer)
                                              .build("Octree", DebugLabelColor::RED));
}

void OctreeRenderer::add_octree(std::weak_ptr<Cube> cube) {
    m_cubes.emplace_back(std::move(cube));
}

void OctreeRenderer::remove_octree(const std::weak_ptr<Cube> cube) {
    m_cubes.erase(std::remove_if(m_cubes.begin(), m_cubes.end(),
                                 [&cube](const std::weak_ptr<Cube> &cube_to_remove) {
                                     return !cube_to_remove.owner_before(cube) && !cube.owner_before(cube_to_remove);
                                 }),
                  m_cubes.end());
}

void OctreeRenderer::set_camera(const std::weak_ptr<Camera> camera) {
    m_camera = camera;
    // TODO: Update mvp matrix!
}

} // namespace inexor::vulkan_renderer::rendering::octree
