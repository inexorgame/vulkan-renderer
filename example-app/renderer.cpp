#include "renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"
#include "standard_ubo.hpp"

#include <cstddef>

namespace inexor::example_app {

// Using declaration
using inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipelineBuilder;

void ExampleAppBase::setup_render_graph() {
    // RENDERGRAPH2
    // @TODO Where to place this? DO we need this here?
    m_render_graph2->reset();

    m_back_buffer = m_render_graph->add<TextureResource>("back buffer", TextureUsage::BACK_BUFFER);
    m_back_buffer->set_format(m_swapchain->image_format());
    // RENDERGRAPH2
    m_back_buffer2 = m_render_graph2->add_texture(
        "back buffer", vulkan_renderer::render_graph::TextureUsage::COLOR_ATTACHMENT, m_swapchain->image_format(),
        m_swapchain->extent().width, m_swapchain->extent().height, 1, VK_SAMPLE_COUNT_1_BIT, [&]() {
            //
        });

    auto *depth_buffer = m_render_graph->add<TextureResource>("depth buffer", TextureUsage::DEPTH_STENCIL_BUFFER);
    depth_buffer->set_format(VK_FORMAT_D32_SFLOAT_S8_UINT);
    // RENDERGRAPH2
    m_depth_buffer2 = m_render_graph2->add_texture(
        "depth buffer", vulkan_renderer::render_graph::TextureUsage::DEPTH_ATTACHMENT, VK_FORMAT_D32_SFLOAT_S8_UINT,
        m_swapchain->extent().width, m_swapchain->extent().height, 1, VK_SAMPLE_COUNT_1_BIT, [&]() {
            //
        });

    m_index_buffer = m_render_graph->add<BufferResource>("index buffer", BufferUsage::INDEX_BUFFER);
    m_index_buffer->upload_data(m_octree_indices);
    // RENDERGRAPH2
    m_index_buffer2 =
        m_render_graph2->add_buffer("index buffer", vulkan_renderer::render_graph::BufferType::INDEX_BUFFER, [&]() {
            // @TODO RENDERGRAPH2 Update the index buffer here
        });

    m_vertex_buffer = m_render_graph->add<BufferResource>("vertex buffer", BufferUsage::VERTEX_BUFFER);
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, position)); // NOLINT
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, color));    // NOLINT
    m_vertex_buffer->upload_data(m_octree_vertices);
    // RENDERGRAPH2
    m_vertex_buffer2 =
        m_render_graph2->add_buffer("vertex buffer", vulkan_renderer::render_graph::BufferType::VERTEX_BUFFER, [&]() {
            // @TODO RENDERGRAPH2 Update the vertex buffer here
        });

    // RENDERGRAPH2
    m_graphics_pass2 = m_render_graph2->get_graphics_pass_builder()
                           .writes_to(m_back_buffer2)
                           .writes_to(m_depth_buffer2)
                           .set_on_record([&](const CommandBuffer &cmd_buf) {
                               // @TODO: bind descriptor set
                               // @TODO: draw indexed
                           })
                           .build("Test", vulkan_renderer::wrapper::DebugLabelColor::GREEN);
    // RENDERGRAPH2
    // Descriptor management for the model/view/projection uniform buffer
    m_render_graph2->add_resource_descriptor(
        [&](vulkan_renderer::wrapper::descriptors::DescriptorSetLayoutBuilder &builder) {
            m_descriptor_set_layout2 = builder
                                           .add(vulkan_renderer::wrapper::descriptors::DescriptorType::UNIFORM_BUFFER,
                                                VK_SHADER_STAGE_VERTEX_BIT)
                                           .build("model/view/proj");
        },
        [&](vulkan_renderer::wrapper::descriptors::DescriptorSetAllocator &allocator) {
            m_descriptor_set2 = allocator.allocate("model/view/proj", m_descriptor_set_layout2);
        },
        [&](vulkan_renderer::wrapper::descriptors::WriteDescriptorSetBuilder &builder) {
            // TODO: Modify to create several descriptor sets (an array?) for each octree
            // TODO: Specify camera matrix as push constant
            // TODO: Multiply view and perspective matrix on cpu and pass as one matrix!
            // TODO: Use one big descriptor (array?) and pass view*perspective and array index as push constant!
            // This will require changes to DescriptorSetLayoutBuilder and more!
            return builder.add(m_descriptor_set2, m_mvp_matrix2).build();
        });

    // @TODO We don't need to re-load the shaders when recreating swapchain!
    // RENDERGRAPH2
    m_vertex_shader2 =
        std::make_shared<Shader>(*m_device, VK_SHADER_STAGE_VERTEX_BIT, "Octree", "shaders/main.vert.spv");
    m_fragment_shader2 =
        std::make_shared<Shader>(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "Octree", "shaders/main.frag.spv");

    // RENDERGRAPH2
    m_render_graph2->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        // RENDERGRAPH2
        m_octree_pipeline2 = builder.add_shader(m_vertex_shader2)
                                 .add_shader(m_fragment_shader2)
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
                                 .set_multisampling(VK_SAMPLE_COUNT_1_BIT)
                                 .add_default_color_blend_attachment()
                                 .add_color_attachment_format(m_back_buffer2.lock()->format())
                                 .set_viewport(m_back_buffer2.lock()->extent())
                                 .set_descriptor_set_layout(m_descriptor_set_layout2)
                                 .build("Octree");
    });

    // RENDERGRAPH2
    m_graphics_pass2 = m_render_graph2->add_graphics_pass(
        // @TODO bind pipeline!
        m_render_graph2->get_graphics_pass_builder()
            .writes_to(m_back_buffer2)
            .writes_to(m_depth_buffer2)
            .set_on_record([&](const CommandBuffer &cmd_buf) {
                cmd_buf.bind_pipeline(m_octree_pipeline2)
                    .bind_descriptor_set(m_descriptor_set2, m_octree_pipeline2)
                    .draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
            })
            .build("Octree Pass", vulkan_renderer::render_graph::DebugLabelColor::GREEN));

    auto *main_stage = m_render_graph->add<GraphicsStage>("main stage");
    main_stage->writes_to(m_back_buffer);
    main_stage->writes_to(depth_buffer);
    main_stage->reads_from(m_index_buffer);
    main_stage->reads_from(m_vertex_buffer);
    main_stage->bind_buffer(m_vertex_buffer, 0);
    main_stage->set_clears_screen(true);
    main_stage->set_depth_options(true, true);
    main_stage->set_on_record([&](const PhysicalStage &physical, const CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor_sets(m_descriptors[0].descriptor_sets(), physical.m_pipeline->pipeline_layout());
        cmd_buf.draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
    });

    for (const auto &shader : m_shaders) {
        main_stage->uses_shader(shader);
    }

    main_stage->add_descriptor_layout(m_descriptors[0].descriptor_set_layout());
}

void ExampleAppBase::recreate_swapchain() {
    m_window->wait_for_focus();
    m_device->wait_idle();

    // Query the framebuffer size here again although the window width is set during framebuffer resize callback
    // The reason for this is that the framebuffer size could already be different again because we missed a poll
    // This seems to be an issue on Linux only though
    auto [window_width, window_height] = m_window->get_framebuffer_size();

    // TODO: This should be abstracted itno a method of the Window wrapper.
    // TODO: This is quite naive, we don't need to recompile the whole render graph on swapchain invalidation.
    m_render_graph.reset();
    // Recreate the swapchain
    m_swapchain->setup_swapchain(
        VkExtent2D{static_cast<std::uint32_t>(window_width), static_cast<std::uint32_t>(window_height)},
        m_vsync_enabled);

    // RENDERGRAPH2
    // Recreate the swapchain
    m_swapchain2->setup_swapchain(
        VkExtent2D{static_cast<std::uint32_t>(window_width), static_cast<std::uint32_t>(window_height)},
        m_vsync_enabled);

    m_render_graph = std::make_unique<RenderGraph>(*m_device, *m_swapchain);
    // RENDERGRAPH2
    m_render_graph2 =
        std::make_unique<inexor::vulkan_renderer::render_graph::RenderGraph>(*m_device, *m_pipeline_cache2);

    setup_render_graph();

    m_camera->set_aspect_ratio(window_width, window_height);

    m_imgui_overlay.reset();

    // RENDERGRAPH2
    m_imgui_overlay = std::make_unique<ImGUIOverlay>(*m_device, *m_swapchain, m_swapchain2, m_render_graph.get(),
                                                     m_back_buffer, m_graphics_pass2, m_render_graph2, []() {
                                                         // RENDERGRAPH2
                                                         // TODO: Implement me!
                                                     });
    m_render_graph->compile(m_back_buffer);

    // RENDERGRAPH2
    m_render_graph2->compile();
}

void ExampleAppBase::render_frame() {
    if (m_window_resized) {
        m_window_resized = false;
        recreate_swapchain();
        return;
    }

    const auto image_index = m_swapchain->acquire_next_image_index();

    const auto &cmd_buf = m_device->request_command_buffer(VulkanQueueType::QUEUE_TYPE_GRAPHICS, "rendergraph");

    m_render_graph->render(image_index, cmd_buf);

    const std::array<VkPipelineStageFlags, 1> stage_mask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    cmd_buf.submit_and_wait(inexor::vulkan_renderer::wrapper::make_info<VkSubmitInfo>({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_swapchain->image_available_semaphore(),
        .pWaitDstStageMask = stage_mask.data(),
        .commandBufferCount = 1,
    }));

    m_swapchain->present(image_index);

    // RENDERGRAPH2
    // @TODO Abstract this into rendergraph!
    const auto img_index2 = m_swapchain2->acquire_next_image_index();
    const auto &cmd_buf2 = m_device->request_command_buffer(VulkanQueueType::QUEUE_TYPE_GRAPHICS, "rendergraph2");
    m_render_graph2->render();
    cmd_buf.submit_and_wait(inexor::vulkan_renderer::wrapper::make_info<VkSubmitInfo>({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_swapchain2->image_available_semaphore(),
        .pWaitDstStageMask = stage_mask.data(),
        .commandBufferCount = 1,
    }));
    m_swapchain2->present(img_index2);

    if (auto fps_value = m_fps_limiter.get_fps()) {
        m_window->set_title("Inexor Vulkan API renderer demo - " + std::to_string(*fps_value) + " FPS");
        spdlog::trace("FPS: {}, window size: {} x {}", *fps_value, m_window->width(), m_window->height());
    }
}

ExampleAppBase::~ExampleAppBase() {
    spdlog::trace("Shutting down vulkan renderer");
    m_device->wait_idle();
}

} // namespace inexor::example_app
