#include "renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "standard_ubo.hpp"

namespace inexor::example_app {

void ExampleAppBase::setup_render_graph() {
    m_back_buffer = m_render_graph->add<TextureResource>("back buffer", TextureUsage::BACK_BUFFER);
    m_back_buffer->set_format(m_swapchain->image_format());

    auto *depth_buffer = m_render_graph->add<TextureResource>("depth buffer", TextureUsage::DEPTH_STENCIL_BUFFER);
    depth_buffer->set_format(VK_FORMAT_D32_SFLOAT_S8_UINT);

    m_index_buffer = m_render_graph->add<BufferResource>("index buffer", BufferUsage::INDEX_BUFFER);
    m_index_buffer->upload_data(m_octree_indices);

    m_vertex_buffer = m_render_graph->add<BufferResource>("vertex buffer", BufferUsage::VERTEX_BUFFER);
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, position)); // NOLINT
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, color));    // NOLINT
    m_vertex_buffer->upload_data(m_octree_vertices);

    auto *main_stage = m_render_graph->add<GraphicsStage>("main stage");
    main_stage->writes_to(m_back_buffer);
    main_stage->writes_to(depth_buffer);
    main_stage->reads_from(m_index_buffer);
    main_stage->reads_from(m_vertex_buffer);
    main_stage->bind_buffer(m_vertex_buffer, 0);
    main_stage->set_clears_screen(true);
    main_stage->set_depth_options(true, true);
    main_stage->set_on_record([&](const PhysicalStage &physical, const CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor_sets(m_descriptors[0].descriptor_sets(), physical.pipeline_layout());
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
    m_render_graph = std::make_unique<RenderGraph>(*m_device, *m_swapchain);
    setup_render_graph();

    m_camera->set_aspect_ratio(window_width, window_height);

    m_imgui_overlay.reset();
    m_imgui_overlay = std::make_unique<ImGUIOverlay>(*m_device, *m_swapchain, m_render_graph.get(), m_back_buffer);
    m_render_graph->compile(m_back_buffer);
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

    if (auto fps_value = m_fps_limiter.get_fps()) {
        m_window->set_title("Inexor Vulkan API renderer demo - " + std::to_string(*fps_value) + " FPS");
        spdlog::trace("FPS: {}, window size: {} x {}", *fps_value, m_window->width(), m_window->height());
    }
}

ExampleAppBase::~ExampleAppBase() {
    spdlog::trace("Shutting down vulkan renderer");
    if (m_device == nullptr) {
        return;
    }
    m_device->wait_idle();
}

} // namespace inexor::example_app
