﻿#include "inexor/vulkan-renderer/renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <array>
#include <fstream>

namespace inexor::vulkan_renderer {

void VulkanRenderer::setup_render_graph() {
    m_back_buffer =
        m_render_graph->add<TextureResource>("back buffer", m_swapchain->image_format(), TextureUsage::BACK_BUFFER);

    m_depth_buffer = m_render_graph->add<TextureResource>("depth buffer", VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                          TextureUsage::DEPTH_STENCIL_BUFFER);

    m_octree_renderer.reset();
    m_octree_renderer = std::make_unique<world::OctreeRenderer<OctreeGpuVertex>>();

    m_octree_gpu_data.reserve(m_worlds.size());

    for (const auto &world : m_worlds) {
        m_octree_gpu_data.emplace_back(m_render_graph.get(), world);
    }

    for (const auto &data : m_octree_gpu_data) {
        m_octree_renderer->setup_stage(m_render_graph.get(), m_back_buffer, m_depth_buffer, m_octree_shaders, data);
    }

    for (const auto &model_file : m_gltf_model_files) {
        glm::mat4 view = m_camera->view_matrix();
        glm::mat4 proj = m_camera->perspective_matrix();
        m_gltf_models.emplace_back(*m_device, m_render_graph.get(), model_file, view, proj);
    }

    m_gltf_model_renderer.reset();
    m_gltf_model_renderer = std::make_unique<gltf::ModelRenderer>();

    for (const auto &model : m_gltf_models) {
        m_gltf_model_renderer->setup_stage(m_render_graph.get(), m_back_buffer, m_depth_buffer, m_octree_shaders,
                                           model);
    }
}

void VulkanRenderer::recreate_swapchain() {
    m_window->wait_for_focus();
    vkDeviceWaitIdle(m_device->device());

    // TODO: This is quite naive, we don't need to recompile the whole render graph on swapchain invalidation.
    m_render_graph.reset();
    m_render_graph = std::make_unique<RenderGraph>(*m_device, m_command_pool->get(), *m_swapchain);

    setup_render_graph();

    m_swapchain->recreate(m_window->width(), m_window->height());

    m_frame_finished_fence.reset();
    m_frame_finished_fence = std::make_unique<wrapper::Fence>(*m_device, "Frame finished fence", true);

    m_image_available_semaphore.reset();
    m_image_available_semaphore = std::make_unique<wrapper::Semaphore>(*m_device, "Image available semaphore");

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

    // Wait for last frame to finish rendering.
    m_frame_finished_fence->block();
    m_frame_finished_fence->reset();

    const auto image_index = m_swapchain->acquire_next_image(*m_image_available_semaphore);
    VkSemaphore wait_semaphore = m_render_graph->render(image_index, m_image_available_semaphore->get(),
                                                        m_device->graphics_queue(), m_frame_finished_fence->get());

    // TODO(): Create a queue wrapper class
    auto present_info = wrapper::make_info<VkPresentInfoKHR>();
    present_info.swapchainCount = 1;
    present_info.waitSemaphoreCount = 1;
    present_info.pImageIndices = &image_index;
    present_info.pSwapchains = m_swapchain->swapchain_ptr();
    present_info.pWaitSemaphores = &wait_semaphore;
    vkQueuePresentKHR(m_device->present_queue(), &present_info);

    if (auto fps_value = m_fps_counter.update()) {
        m_window->set_title("Inexor Vulkan API renderer demo - " + std::to_string(*fps_value) + " FPS");
        spdlog::debug("FPS: {}, window size: {} x {}.", *fps_value, m_window->width(), m_window->height());
    }
}

void VulkanRenderer::calculate_memory_budget() {
    VmaStats memory_stats;
    vmaCalculateStats(m_device->allocator(), &memory_stats);

    spdlog::debug("-------------VMA stats-------------");
    spdlog::debug("Number of `VkDeviceMemory` (physical memory) blocks allocated: {} still alive, {} in total",
                  memory_stats.memoryHeap->blockCount, memory_stats.total.blockCount);
    spdlog::debug("Number of VmaAlllocation objects allocated (requested memory): {} still alive, {} in total",
                  memory_stats.memoryHeap->allocationCount, memory_stats.total.allocationCount);
    spdlog::debug("Number of free ranges of memory between allocations: {}", memory_stats.memoryHeap->unusedRangeCount);
    spdlog::debug("Total number of bytes occupied by all allocations: {}", memory_stats.memoryHeap->usedBytes);
    spdlog::debug("Total number of bytes occupied by unused ranges: {}", memory_stats.memoryHeap->unusedBytes);
    spdlog::debug("memory_stats.memoryHeap->allocationSizeMin: {}", memory_stats.memoryHeap->allocationSizeMin);
    spdlog::debug("memory_stats.memoryHeap->allocationSizeAvg: {}", memory_stats.memoryHeap->allocationSizeAvg);
    spdlog::debug("memory_stats.memoryHeap->allocationSizeMax: {}", memory_stats.memoryHeap->allocationSizeMax);
    spdlog::debug("memory_stats.memoryHeap->unusedRangeSizeMin: {}", memory_stats.memoryHeap->unusedRangeSizeMin);
    spdlog::debug("memory_stats.memoryHeap->unusedRangeSizeAvg: {}", memory_stats.memoryHeap->unusedRangeSizeAvg);
    spdlog::debug("memory_stats.memoryHeap->unusedRangeSizeMax: {}", memory_stats.memoryHeap->unusedRangeSizeMax);
    spdlog::debug("-------------VMA stats-------------");

    char *vma_stats_string = nullptr;
    vmaBuildStatsString(m_device->allocator(), &vma_stats_string, VK_TRUE);

    std::string memory_dump_file_name = "vma-dumps/dump.json";
    std::ofstream vma_memory_dump(memory_dump_file_name, std::ios::out);
    vma_memory_dump.write(vma_stats_string, strlen(vma_stats_string)); // NOLINT
    vma_memory_dump.close();

    vmaFreeStatsString(m_device->allocator(), vma_stats_string);
}

VulkanRenderer::~VulkanRenderer() {
    spdlog::debug("Shutting down vulkan renderer");
    // TODO: Add wrapper::Device::wait_idle()
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
