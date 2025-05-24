#include "inexor/vulkan-renderer/render-modules/octree_renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/exception.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_modules {

// Using declarations
using wrapper::InexorException;
using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::WriteDescriptorSetBuilder;

OctreeRenderer::OctreeRenderer(const Device &device,
                               const std::weak_ptr<TextureResource> back_buffer,
                               const std::weak_ptr<TextureResource> depth_buffer)
    : RenderModuleBase(device, "OctreeRenderer") {
    if (back_buffer.expired()) {
        throw InexorException("Error: Parameter 'back_buffer' is an invalid pointer!");
    }
    if (depth_buffer.expired()) {
        throw InexorException("Error: Parameter 'depth_buffer' is an invalid pointer!");
    }
    m_back_buffer = back_buffer;
    m_depth_buffer = depth_buffer;
}

void OctreeRenderer::setup_buffers() {
    // TODO: Refactor so the view*proj matrix is a push constant range and every octree has its own uniform buffer
    // TODO: Use dynamic UBO with offsets or some other alternative
    m_camera_matrix = std::make_shared<Buffer>("model/view/proj", BufferType::UNIFORM_BUFFER);
    m_buffers.push_back(m_camera_matrix);
}

void OctreeRenderer::setup_graphics_passes(GraphicsPassBuilder &builder) {
    // TODO: We need to specify from which buffer(s) the graphics pass (or module) reads so rendergraph knows about it!
    auto octree_graphics_pass =
        builder
            .set_on_record([&](const CommandBuffer &cmd_buf) {
                cmd_buf.bind_pipeline(m_graphics_pipelines[0]);
                // Render all the cubes which are registered to the octree renderer
                for (const auto &cube : m_cubes) {
                    auto cb = cube.lock();
                    // TODO: Merge all vertex buffers of same type into one big buffer and use offsets when
                    // drawing?
                    cmd_buf.bind_vertex_buffer(cb->m_vertex_buffer)
                        .bind_index_buffer(cb->m_index_buffer)
                        // TODO: Do we need to bind descriptor set in the loop or only once before?
                        .bind_descriptor_set(m_descriptor_set, m_graphics_pipelines[0])
                        .draw_indexed(cb->m_index_count);
                }
            })
            .writes_to(m_back_buffer)
            .writes_to(m_depth_buffer)
            .build("Octree", DebugLabelColor::RED);

    // Add the new octree graphics pass to the list of graphics passes of this render module
    m_graphics_passes.emplace_back(std::move(octree_graphics_pass));
}

void OctreeRenderer::setup_graphics_pipelines(GraphicsPipelineBuilder &builder) {
    // Store the image format and extent of the back buffer
    m_back_buffer_img_format = m_back_buffer.lock()->format();
    m_back_buffer_extent = m_back_buffer.lock()->extent();

    // Create the octree graphics pipeline using the graphics pipeline builder
    auto octree_graphics_pipeline = builder.add_shaders(m_shaders)
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
                                        .set_descriptor_set_layout(m_descriptor_set_layouts[0])
                                        .build("Octree");

    // By using the graphics pipeline builder, add the octree graphics pipeline
    m_graphics_pipelines.emplace_back(std::move(octree_graphics_pipeline));
}

void OctreeRenderer::update_buffers() {
    m_camera_matrix->request_update(m_camera.lock()->get_view_perspective_matrix());
}

void OctreeRenderer::update_textures() {
    // OctreeRenderer does not need to update any textures because the texture that is loaded at startup doesn't change
    // NOTE: We are still forced to explicitely implement update_textures() when inheriting from RenderModuleBase
}

void OctreeRenderer::allocate_descriptor_sets(DescriptorSetAllocator &allocator) {
    m_descriptor_set = allocator.allocate("model/view/proj", m_descriptor_set_layouts[0]);
}

void OctreeRenderer::update_descriptor_sets(WriteDescriptorSetBuilder &builder) {
    builder.add(m_descriptor_set, m_camera_matrix);
}

void OctreeRenderer::setup_descriptor_set_layouts(DescriptorSetLayoutBuilder &builder) {
    using wrapper::descriptors::DescriptorType;
    m_descriptor_set_layouts.emplace_back(
        builder.add(DescriptorType::UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).build("model/view/proj"));
}

void OctreeRenderer::setup_shaders() {
    m_shaders.emplace_back(m_device, "Octree|vert", VK_SHADER_STAGE_VERTEX_BIT, "shaders/octree.vert.spv");
    m_shaders.emplace_back(m_device, "Octree|frag", VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/octree.frag.spv");
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

} // namespace inexor::vulkan_renderer::render_modules
