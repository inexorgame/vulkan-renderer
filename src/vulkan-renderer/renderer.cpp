#include "inexor/vulkan-renderer/renderer.hpp"

#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/wrapper/info.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <array>
#include <fstream>

namespace inexor::vulkan_renderer {

void VulkanRenderer::setup_frame_graph() {
    auto &back_buffer = m_frame_graph->add<TextureResource>("back buffer");
    back_buffer.set_format(m_swapchain->image_format());
    back_buffer.set_usage(TextureUsage::BACK_BUFFER);

    auto &depth_buffer = m_frame_graph->add<TextureResource>("depth buffer");
    depth_buffer.set_format(VK_FORMAT_D32_SFLOAT_S8_UINT);
    depth_buffer.set_usage(TextureUsage::DEPTH_STENCIL_BUFFER);

    auto &vertex_buffer = m_frame_graph->add<BufferResource>("vertex buffer");
    vertex_buffer.set_usage(BufferUsage::VERTEX_BUFFER);
    vertex_buffer.add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, position));
    vertex_buffer.add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, color));
    vertex_buffer.upload_data(m_octree_vertices);

    auto &main_stage = m_frame_graph->add<GraphicsStage>("main stage");
    main_stage.writes_to(back_buffer);
    main_stage.writes_to(depth_buffer);
    main_stage.reads_from(vertex_buffer);
    main_stage.add_descriptor_layout(m_descriptors[0].descriptor_set_layout());
    main_stage.bind_buffer(vertex_buffer, 0);
    main_stage.set_clears_screen(true);
    main_stage.set_on_record([&](const PhysicalStage *phys, const wrapper::CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor(m_descriptors[0], phys->pipeline_layout());
        cmd_buf.draw(m_octree_vertices.size());
    });
    for (const auto &shader : m_shaders) {
        main_stage.uses_shader(shader);
    }

    auto &ui_stage = m_frame_graph->add<GraphicsStage>("imgui stage");
    ui_stage.writes_to(back_buffer);
    ui_stage.add_descriptor_layout(m_descriptors[1].descriptor_set_layout());
    for (const auto &shader : m_ui_shaders) {
        ui_stage.uses_shader(shader);
    }

    m_frame_graph->compile(back_buffer);
}

void VulkanRenderer::setup_ui() {
    auto &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("assets/fonts/vegur/vegur.otf", 16.0f);

    unsigned char *font_data = nullptr;
    int texture_width = 0;
    int texture_height = 0;
    io.Fonts->GetTexDataAsRGBA32(&font_data, &texture_width, &texture_height);

    // Create a GPU buffer and upload the texture
    // NOTE: * 4 for 4 channels
    VkDeviceSize texture_size = texture_width * texture_height * 4 * sizeof(*font_data);
    m_imgui_texture = std::make_unique<wrapper::Texture>(
        m_vkdevice->device(), m_vkdevice->physical_device(), m_vkdevice->allocator(), font_data, texture_size,
        "imgui_overlay", m_vkdevice->transfer_queue(), m_vkdevice->transfer_queue_family_index());

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings(2);
    std::vector<VkWriteDescriptorSet> descriptor_writes(2);
    layout_bindings[0].binding = 0;
    layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_bindings[0].descriptorCount = 1;
    layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_bindings[1].binding = 1;
    layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_bindings[1].descriptorCount = 1;
    layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    ui_ubo_info.buffer = m_uniform_buffers[1].buffer();
    ui_ubo_info.offset = 0;
    ui_ubo_info.range = sizeof(UiUniformBufferObject);

    // Setup UBO
    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pBufferInfo = &ui_ubo_info;

    // Setup sampler
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = m_imgui_texture->image_view();
    image_info.sampler = m_imgui_texture->sampler();
    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].pImageInfo = &image_info;
    m_descriptors.emplace_back(
        wrapper::ResourceDescriptor{m_vkdevice->device(),
                                    m_swapchain->image_count(),
                                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
                                    layout_bindings,
                                    descriptor_writes,
                                    "UI descriptor"});
}

void VulkanRenderer::recreate_swapchain() {
    m_window->wait_for_focus();
    vkDeviceWaitIdle(m_vkdevice->device());

    // TODO(): This is quite naive, we don't need to recompile the whole frame graph on swapchain invalidation
    m_frame_graph.reset();
    m_swapchain->recreate(m_window->width(), m_window->height());
    m_frame_graph = std::make_unique<FrameGraph>(m_vkdevice->device(), m_command_pool->get(), m_vkdevice->allocator(),
                                                 *m_swapchain);
    setup_frame_graph();

    m_image_available_semaphore.reset();
    m_rendering_finished_semaphore.reset();
    m_image_available_semaphore =
        std::make_unique<wrapper::Semaphore>(m_vkdevice->device(), "Image available semaphore");
    m_rendering_finished_semaphore =
        std::make_unique<wrapper::Semaphore>(m_vkdevice->device(), "Rendering finished semaphore");
    vkDeviceWaitIdle(m_vkdevice->device());

    m_game_camera.m_type = Camera::CameraType::LOOKAT;
    m_game_camera.m_rotation_speed = 0.25f;
    m_game_camera.m_movement_speed = 0.1f;
    m_game_camera.set_position({0.0f, 0.0f, 5.0f});
    m_game_camera.set_rotation({0.0f, 0.0f, 0.0f});
    m_game_camera.set_perspective(45.0f, static_cast<float>(m_window->width()) / static_cast<float>(m_window->height()),
                                  0.1f, 256.0f);
}

void VulkanRenderer::render_frame() {
    if (m_window_resized) {
        m_window_resized = false;
        recreate_swapchain();
        return;
    }

    const auto image_index = m_swapchain->acquire_next_image(*m_image_available_semaphore);
    m_frame_graph->render(image_index, m_rendering_finished_semaphore->get(), m_image_available_semaphore->get(),
                          m_vkdevice->graphics_queue());

    // TODO(): Create a queue wrapper class
    auto present_info = wrapper::make_info<VkPresentInfoKHR>();
    present_info.swapchainCount = 1;
    present_info.waitSemaphoreCount = 1;
    present_info.pImageIndices = &image_index;
    present_info.pSwapchains = m_swapchain->swapchain_ptr();
    present_info.pWaitSemaphores = m_rendering_finished_semaphore->ptr();
    vkQueuePresentKHR(m_vkdevice->present_queue(), &present_info);

    if (auto fps_value = m_fps_counter.update()) {
        m_window->set_title("Inexor Vulkan API renderer demo - " + std::to_string(*fps_value) + " FPS");
        spdlog::debug("FPS: {}, window size: {} x {}.", *fps_value, m_window->width(), m_window->height());
    }
}

void VulkanRenderer::calculate_memory_budget() {
    VmaStats memory_stats;
    vmaCalculateStats(m_vkdevice->allocator(), &memory_stats);

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
    vmaBuildStatsString(m_vkdevice->allocator(), &vma_stats_string, VK_TRUE);

    std::string memory_dump_file_name = "vma-dumps/dump.json";
    std::ofstream vma_memory_dump(memory_dump_file_name, std::ios::out);
    vma_memory_dump.write(vma_stats_string, strlen(vma_stats_string));
    vma_memory_dump.close();

    vmaFreeStatsString(m_vkdevice->allocator(), vma_stats_string);
}

VulkanRenderer::~VulkanRenderer() {
    spdlog::debug("Shutting down vulkan renderer");
    // TODO: Add wrapper::Device::wait_idle()
    vkDeviceWaitIdle(m_vkdevice->device());

    if (!m_debug_report_callback_initialised) {
        return;
    }

    // TODO(): Is there a better way to do this? Maybe add a helper function to wrapper::Instance?
    auto vk_destroy_debug_report_callback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(m_vkinstance->instance(), "vkDestroyDebugReportCallbackEXT"));
    if (vk_destroy_debug_report_callback != nullptr) {
        vk_destroy_debug_report_callback(m_vkinstance->instance(), m_debug_report_callback, nullptr);
    }
}

} // namespace inexor::vulkan_renderer
