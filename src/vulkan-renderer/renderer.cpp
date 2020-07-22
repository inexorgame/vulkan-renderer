#include "inexor/vulkan-renderer/renderer.hpp"

#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/octree_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/wrapper/info.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <fstream>

namespace inexor::vulkan_renderer {

void VulkanRenderer::setup_frame_graph() {
    auto &back_buffer = m_frame_graph->add<TextureResource>("back buffer");
    back_buffer.set_format(swapchain->get_image_format());
    back_buffer.set_usage(TextureUsage::BACK_BUFFER);

    auto &depth_buffer = m_frame_graph->add<TextureResource>("depth buffer");
    depth_buffer.set_format(VK_FORMAT_D32_SFLOAT_S8_UINT);
    depth_buffer.set_usage(TextureUsage::DEPTH_STENCIL_BUFFER);

    auto &main_stage = m_frame_graph->add<GraphicsStage>("main stage");
    main_stage.writes_to(back_buffer);
    main_stage.writes_to(depth_buffer);
    main_stage.set_on_record([&](const PhysicalStage *phys, const wrapper::CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor(descriptors[0], phys->pipeline_layout());
        cmd_buf.bind_vertex_buffer(mesh_buffers[0].get_vertex_buffer());
        cmd_buf.draw(mesh_buffers[0].get_vertex_count());
    });

    for (const auto &shader : shaders) {
        main_stage.uses_shader(shader);
    }

    for (const auto &attribute_binding : OctreeVertex::get_attribute_binding_description()) {
        main_stage.add_attribute_binding(attribute_binding);
    }

    main_stage.add_descriptor_layout(descriptors[0].get_descriptor_set_layout());
    main_stage.add_vertex_binding(OctreeVertex::get_vertex_binding_description());
    m_frame_graph->compile(back_buffer);
}

VkResult VulkanRenderer::create_synchronisation_objects() {
    assert(swapchain->get_image_count() > 0);

    spdlog::debug("Creating synchronisation objects: semaphores and fences.");
    spdlog::debug("Number of images in swapchain: {}.", swapchain->get_image_count());

    // TODO: Add method to create several fences/semaphores.

    image_available_semaphore =
        std::make_unique<wrapper::Semaphore>(vkdevice->get_device(), "Image available semaphore");
    rendering_finished_semaphore =
        std::make_unique<wrapper::Semaphore>(vkdevice->get_device(), "Rendering finished semaphore");

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_descriptor_pool() {
    descriptors.emplace_back(vkdevice->get_device(), swapchain->get_image_count(), std::string("unnamed descriptor"));

    // Create the descriptor pool.
    descriptors[0].create_descriptor_pool(
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_descriptor_set_layouts() {
    std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings(2);

    descriptor_set_layout_bindings[0].binding = 0;
    descriptor_set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_set_layout_bindings[0].descriptorCount = 1;
    descriptor_set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptor_set_layout_bindings[0].pImmutableSamplers = nullptr;

    descriptor_set_layout_bindings[1].binding = 1;
    descriptor_set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_set_layout_bindings[1].descriptorCount = 1;
    descriptor_set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptor_set_layout_bindings[1].pImmutableSamplers = nullptr;

    descriptors[0].create_descriptor_set_layouts(descriptor_set_layout_bindings);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_descriptor_writes() {
    assert(!textures.empty());

    std::vector<VkWriteDescriptorSet> descriptor_writes(2);

    // Link the matrices uniform buffer to the descriptor set so the shader can access it.

    // We can do better than this, but therefore RAII refactoring needs to be done..
    uniform_buffer_info.buffer = uniform_buffers[0].get_buffer();
    uniform_buffer_info.offset = 0;
    uniform_buffer_info.range = sizeof(UniformBufferObject);

    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = nullptr;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pBufferInfo = &uniform_buffer_info;

    // Link the texture to the descriptor set so the shader can access it.
    descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptor_image_info.imageView = textures[0].get_image_view();
    descriptor_image_info.sampler = textures[0].get_sampler();

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = nullptr;
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].pImageInfo = &descriptor_image_info;

    descriptors[0].add_descriptor_writes(descriptor_writes);

    descriptors[0].create_descriptor_sets();

    return VK_SUCCESS;
}

void VulkanRenderer::render_frame() {
    // TODO(): Add std::uint32_t wrapper::Swapchain::acquire_next_image(const wrapper::Semaphore &)
    std::uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(vkdevice->get_device(), swapchain->get_swapchain(), UINT64_MAX,
                                            image_available_semaphore->get(), VK_NULL_HANDLE, &image_index);
    assert(result == VK_SUCCESS);

    m_frame_graph->render(image_index, rendering_finished_semaphore->get(), image_available_semaphore->get(),
                          vkdevice->get_graphics_queue());

    // TODO(): Create a queue wrapper class
    auto present_info = wrapper::make_info<VkPresentInfoKHR>();
    present_info.swapchainCount = 1;
    present_info.waitSemaphoreCount = 1;
    present_info.pImageIndices = &image_index;
    present_info.pSwapchains = swapchain->get_swapchain_ptr();
    present_info.pWaitSemaphores = rendering_finished_semaphore->get_ptr();
    vkQueuePresentKHR(vkdevice->get_present_queue(), &present_info);

    if (auto fps_value = fps_counter.update()) {
        window->set_title("Inexor Vulkan API renderer demo - " + std::to_string(*fps_value) + " FPS");
        spdlog::debug("FPS: {}, window size: {} x {}.", *fps_value, window->get_width(), window->get_height());
    }
}

void VulkanRenderer::calculate_memory_budget() {
    VmaStats memory_stats;
    vmaCalculateStats(vkdevice->allocator(), &memory_stats);

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
    vmaBuildStatsString(vkdevice->allocator(), &vma_stats_string, VK_TRUE);

    std::string memory_dump_file_name = "vma-dumps/dump.json";
    std::ofstream vma_memory_dump(memory_dump_file_name, std::ios::out);
    vma_memory_dump.write(vma_stats_string, strlen(vma_stats_string));
    vma_memory_dump.close();

    vmaFreeStatsString(vkdevice->allocator(), vma_stats_string);
}

VkResult VulkanRenderer::shutdown_vulkan() {
    // It is important to destroy the objects in reversal of the order of creation.
    spdlog::debug("Shutting down Vulkan API.");

    vkDeviceWaitIdle(vkdevice->get_device());
    m_frame_graph.reset();

    // @todo: (yeetari) Remove once this class is RAII-ified.
    shaders.clear();
    textures.clear();
    uniform_buffers.clear();
    mesh_buffers.clear();
    descriptors.clear();

    image_available_semaphore.reset();
    rendering_finished_semaphore.reset();

    // @todo: (Hanni) Remove them once this class is RAII-ified.
    command_pool.reset();
    swapchain.reset();
    surface.reset();
    vkdevice.reset();

    // Destroy Vulkan debug callback.
    if (debug_report_callback_initialised) {
        // We have to explicitly load this function.
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
            reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                vkGetInstanceProcAddr(vkinstance->get_instance(), "vkDestroyDebugReportCallbackEXT"));

        if (nullptr != vkDestroyDebugReportCallbackEXT) {
            vkDestroyDebugReportCallbackEXT(vkinstance->get_instance(), debug_report_callback, nullptr);
            debug_report_callback_initialised = false;
        }
    }

    spdlog::debug("Shutdown finished.");
    spdlog::debug(
        "------------------------------------------------------------------------------------------------------------");

    vkinstance.reset();

    return VK_SUCCESS;
}

} // namespace inexor::vulkan_renderer
