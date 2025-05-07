#include "inexor/vulkan-renderer/render-components/octree/octree_renderer.hpp"

#include "inexor/vulkan-renderer/render-components/octree/octree_vertices.hpp"
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

    // NOTE: We already checked if rendergraph is a valid pointer earlier, we don't need to check here if rg is valid
    auto rg = rendergraph.lock();

    using wrapper::descriptors::DescriptorType;

    // Descriptor management for the model/view/projection uniform buffer
    rg->add_resource_descriptor(
        [&](DescriptorSetLayoutBuilder &builder) {
            m_descriptor_set_layout =
                builder.add(DescriptorType::UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).build("model/view/proj");
        },
        [&](DescriptorSetAllocator &allocator) {
            m_descriptor_set = allocator.allocate("model/view/proj", m_descriptor_set_layout);
        },
        [&](WriteDescriptorSetBuilder &builder) {
            // TODO: Modify to create several descriptor sets (an array?) for each octree
            // TODO: Specify camera matrix as push constant
            // TODO: Multiply view and perspective matrix on cpu and pass as one matrix!
            // TODO: Use one big descriptor (array?) and pass view*perspective and array index as push constant!
            // This will require changes to DescriptorSetLayoutBuilder and more!
            return builder.add(m_descriptor_set, m_mvp_matrix).build();
        });

    // TODO: Replace with push_constant_range!
    // Setup uniform buffer for model/view/projection matrix
    m_mvp_matrix = rg->add_buffer("model/view/proj", BufferType::UNIFORM_BUFFER, [&]() {
        const auto camera = m_camera.lock();

        m_mvp_data.model = glm::mat4(1.0f);
        m_mvp_data.view = camera->view_matrix();
        m_mvp_data.proj = camera->perspective_matrix();
        m_mvp_data.proj[1][1] *= -1;

        // Update the uniform buffer for model/view/projection matrix
        m_mvp_matrix.lock()->request_update(m_mvp_data);
    });

    // Load the shaders used for octree rendering
    m_octree_fragment =
        std::make_shared<Shader>(rg->device(), "Octree|vert", VK_SHADER_STAGE_VERTEX_BIT, "shaders/octree.vert.spv");
    m_octree_vertex =
        std::make_shared<Shader>(rg->device(), "Octree|frag", VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/octree.frag.spv");

    // Store the image format and extent of the back buffer
    m_back_buffer_img_format = back_buffer.lock()->format();
    m_back_buffer_extent = back_buffer.lock()->extent();

    // Set the callback for creation of the graphics pipeline
    rg->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        m_octree_pipeline = builder.add_shader(m_octree_vertex)
                                .add_shader(m_octree_fragment)
                                .set_vertex_input_bindings({{
                                    .binding = 0,
                                    .stride = sizeof(SimpleColoredVertex),
                                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                                }})
                                .set_vertex_input_attributes({
                                    {
                                        .location = 0,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(SimpleColoredVertex, position),
                                    },
                                    {
                                        .location = 1,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(SimpleColoredVertex, color),
                                    },
                                })
                                .set_scissor(m_back_buffer_extent)
                                .set_viewport(m_back_buffer_extent)
                                .add_default_color_blend_attachment()
                                .set_multisampling(VK_SAMPLE_COUNT_1_BIT)
                                .add_color_attachment_format(m_back_buffer_img_format)
                                .set_descriptor_set_layout(m_descriptor_set_layout)
                                .build("Octree");
    });

    // Add the graphics pass for octree rendering
    m_octree_pass = rg->add_graphics_pass(
        rg->get_graphics_pass_builder()
            // TODO: We need to specify from which buffer(s) the graphics pass reads so rendergraph knows about it!
            .set_on_record([&](const CommandBuffer &cmd_buf) {
                // NOTE: This is one of those cases where we only have 1 pipeline per stage, meaning that in principle
                // we could bind the pipeline in the rendergraph automatically before invoking the set_on_record command
                // buffer recording function. However, to stay consistent, we should not bind pipelines in rendergraph
                // automatically, since we explicitely mention that it's the job of the programmer to do this manually!
                // The pipeline must be bound only once for rendering all octrees
                cmd_buf.bind_pipeline(m_octree_pipeline);
                // Render all the cubes which are registered to the octree renderer
                for (const auto &cube : m_cubes) {
                    auto cb = cube.lock();
                    // TODO: Merge all vertex buffers of same type into one big buffer and use offsets when drawing?
                    cmd_buf.bind_vertex_buffer(cb->m_vertex_buffer)
                        .bind_index_buffer(cb->m_index_buffer)
                        // TODO: Do we need to bind descriptor set in the loop or only once before?
                        .bind_descriptor_set(m_descriptor_set, m_octree_pipeline)
                        .draw_indexed(cb->m_index_count);
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

void OctreeRenderer::set_camera(std::weak_ptr<Camera> camera) {
    m_camera = camera;
}

} // namespace inexor::vulkan_renderer::rendering::octree
