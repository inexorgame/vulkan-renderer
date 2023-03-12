﻿#include "inexor/vulkan-renderer/renderer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cassert>
#include <fstream>
#include <limits>
#include <unordered_map>

namespace inexor::vulkan_renderer {

void VulkanRenderer::setup_render_graph() {
    m_back_buffer =
        m_render_graph->add<TextureResource>("back buffer", TextureUsage::BACK_BUFFER, m_swapchain->image_format());

    auto *depth_buffer = m_render_graph->add<TextureResource>("depth buffer", TextureUsage::DEPTH_STENCIL_BUFFER,
                                                              VK_FORMAT_D32_SFLOAT_S8_UINT);

    m_index_buffer = m_render_graph->add<BufferResource>("index buffer", BufferUsage::INDEX_BUFFER);
    m_index_buffer->upload_data(m_octree_indices);

    m_vertex_buffer = m_render_graph->add<BufferResource>("vertex buffer", BufferUsage::VERTEX_BUFFER);

    m_vertex_buffer
        ->set_vertex_attributes<OctreeGpuVertex>({
            {.location = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(OctreeGpuVertex, position)},
            {.location = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(OctreeGpuVertex, color)},
        })
        ->upload_data(m_octree_vertices);

    const auto *main_stage =
        m_render_graph->add<GraphicsStage>("Octree")
            ->bind_buffer(m_vertex_buffer, 0)
            ->uses_shaders(m_shaders)
            ->set_clears_screen(true)
            ->set_depth_options(true, true)
            ->writes_to(m_back_buffer)
            ->writes_to(depth_buffer)
            ->reads_from(m_index_buffer)
            ->reads_from(m_vertex_buffer)
            ->set_on_record([&](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
                cmd_buf.bind_descriptor_sets(m_descriptors[0].descriptor_sets(), physical.pipeline_layout());
                cmd_buf.draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
            })
            ->add_descriptor_layout(m_descriptors[0].descriptor_set_layout());
}

void VulkanRenderer::generate_octree_indices() {
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
}

void VulkanRenderer::recreate_swapchain() {
    m_window->wait_for_focus();
    vkDeviceWaitIdle(m_device->device());

    // TODO: This is quite naive, we don't need to recompile the whole render graph on swapchain invalidation.
    m_render_graph.reset();
    m_swapchain->recreate(m_window->width(), m_window->height(), m_vsync_enabled);
    m_render_graph = std::make_unique<RenderGraph>(*m_device, *m_swapchain);
    setup_render_graph();

    m_camera = std::make_unique<Camera>(glm::vec3(6.0f, 10.0f, 2.0f), 180.0f, 0.0f,
                                        static_cast<float>(m_window->width()), static_cast<float>(m_window->height()));

    m_camera->set_movement_speed(5.0f);
    m_camera->set_rotation_speed(0.5f);

    m_imgui_overlay.reset();
    m_imgui_overlay = std::make_unique<ImGUIOverlay>(*m_device, *m_swapchain, m_render_graph.get(), m_back_buffer);
    m_render_graph->compile(m_back_buffer);
}

void VulkanRenderer::render_frame() {
    if (m_window_resized) {
        m_window_resized = false;
        recreate_swapchain();
        return;
    }

    const auto image_index = m_swapchain->acquire_next_image_index();
    const auto &cmd_buf = m_device->request_command_buffer("rendergraph");

    m_render_graph->render(image_index, cmd_buf);

    const std::array<VkPipelineStageFlags, 1> stage_mask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    cmd_buf.submit_and_wait(wrapper::make_info<VkSubmitInfo>({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_swapchain->image_available_semaphore(),
        .pWaitDstStageMask = stage_mask.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = cmd_buf.ptr(),
    }));

    const auto present_info = wrapper::make_info<VkPresentInfoKHR>({
        .swapchainCount = 1,
        .pSwapchains = m_swapchain->swapchain(),
        .pImageIndices = &image_index,
    });

    if (const auto result = vkQueuePresentKHR(m_device->present_queue(), &present_info); result != VK_SUCCESS) {
        throw VulkanException("Error: vkQueuePresentKHR failed!", result);
    }

    if (auto fps_value = m_fps_counter.update()) {
        m_window->set_title("Inexor Vulkan API renderer demo - " + std::to_string(*fps_value) + " FPS");
        spdlog::trace("FPS: {}, window size: {} x {}", *fps_value, m_window->width(), m_window->height());
    }
}

VulkanRenderer::~VulkanRenderer() {
    spdlog::trace("Shutting down vulkan renderer");

    if (m_device == nullptr) {
        return;
    }

    vkDeviceWaitIdle(m_device->device());

    if (!m_debug_report_callback_initialised) {
        return;
    }

    // TODO(): Is there a better way to do this? Maybe add a helper function to wrapper::Instance?
    auto vk_destroy_debug_report_callback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>( // NOLINT
        vkGetInstanceProcAddr(m_instance->instance(), "vkDestroyDebugReportCallbackEXT"));
    if (vk_destroy_debug_report_callback != nullptr) {
        vk_destroy_debug_report_callback(m_instance->instance(), m_debug_report_callback, nullptr);
    }
}

} // namespace inexor::vulkan_renderer
