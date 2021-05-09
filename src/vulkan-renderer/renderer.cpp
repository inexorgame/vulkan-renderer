#include "inexor/vulkan-renderer/renderer.hpp"

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
    auto &back_buffer = m_render_graph->add<TextureResource>("back buffer", TextureUsage::BACK_BUFFER);
    back_buffer.set_format(m_swapchain->image_format());

    auto &depth_buffer = m_render_graph->add<TextureResource>("depth buffer", TextureUsage::DEPTH_STENCIL_BUFFER);
    depth_buffer.set_format(VK_FORMAT_D32_SFLOAT_S8_UINT);

    auto &index_buffer = m_render_graph->add<BufferResource>("index buffer", BufferUsage::INDEX_BUFFER);
    index_buffer.upload_data(m_octree_indices);

    auto &vertex_buffer = m_render_graph->add<BufferResource>("vertex buffer", BufferUsage::VERTEX_BUFFER);
    vertex_buffer.add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, position)); // NOLINT
    vertex_buffer.add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, color));    // NOLINT
    vertex_buffer.upload_data(m_octree_vertices);

    auto &main_stage = m_render_graph->add<GraphicsStage>("main stage");
    main_stage.writes_to(back_buffer);
    main_stage.writes_to(depth_buffer);
    main_stage.reads_from(index_buffer);
    main_stage.reads_from(vertex_buffer);
    main_stage.bind_buffer(vertex_buffer, 0);
    main_stage.set_clears_screen(true);
    main_stage.set_on_record([&](const PhysicalStage *phys, const wrapper::CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor(m_descriptors[0], phys->pipeline_layout());
        cmd_buf.draw_indexed(m_octree_indices.size());
    });

    for (const auto &shader : m_shaders) {
        main_stage.uses_shader(shader);
    }

    main_stage.add_descriptor_layout(m_descriptors[0].descriptor_set_layout());
    m_render_graph->compile(back_buffer);
}

void VulkanRenderer::generate_octree_indices() {
    const auto old_vertex_count = m_octree_vertices.size();
    auto old_vertices = std::move(m_octree_vertices);
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
    m_swapchain->recreate(m_window->width(), m_window->height());
    m_render_graph = std::make_unique<RenderGraph>(*m_device, m_command_pool->get(), *m_swapchain);
    setup_render_graph();

    m_image_available_semaphore.reset();
    m_rendering_finished_semaphore.reset();
    m_image_available_semaphore = std::make_unique<wrapper::Semaphore>(*m_device, "Image available semaphore");
    m_rendering_finished_semaphore = std::make_unique<wrapper::Semaphore>(*m_device, "Rendering finished semaphore");

    m_camera = std::make_unique<Camera>(glm::vec3(5.0f, 5.0f, 10.0f), 250.0f, 0.0f,
                                        static_cast<float>(m_window->width()), static_cast<float>(m_window->height()));

    m_camera->set_movement_speed(5.0f);
    m_camera->set_rotation_speed(0.5f);

    m_imgui_overlay.reset();
    m_imgui_overlay = std::make_unique<ImGUIOverlay>(*m_device, *m_swapchain);
}

void VulkanRenderer::render_frame() {
    if (m_window_resized) {
        m_window_resized = false;
        recreate_swapchain();
        return;
    }

    const auto image_index = m_swapchain->acquire_next_image(*m_image_available_semaphore);
    m_render_graph->render(static_cast<int>(image_index), m_rendering_finished_semaphore->get(),
                           m_image_available_semaphore->get(), m_device->graphics_queue());

    m_imgui_overlay->render(image_index);

    // TODO(): Create a queue wrapper class
    auto present_info = wrapper::make_info<VkPresentInfoKHR>();
    present_info.swapchainCount = 1;
    present_info.waitSemaphoreCount = 1;
    present_info.pImageIndices = &image_index;
    present_info.pSwapchains = m_swapchain->swapchain_ptr();
    present_info.pWaitSemaphores = m_rendering_finished_semaphore->ptr();

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
