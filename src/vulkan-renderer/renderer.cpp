#include "inexor/vulkan-renderer/renderer.hpp"

#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/octree_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <fstream>

namespace inexor::vulkan_renderer {

void VulkanRenderer::create_frame_graph() {
    auto &back_buffer = m_frame_graph.add<TextureResource>("back buffer");
    back_buffer.set_format(swapchain->get_image_format());
    back_buffer.set_usage(TextureUsage::BACK_BUFFER);

    auto &depth_buffer = m_frame_graph.add<TextureResource>("depth buffer");
    depth_buffer.set_format(VK_FORMAT_D32_SFLOAT);
    depth_buffer.set_usage(TextureUsage::DEPTH_BUFFER);

    auto &main_stage = m_frame_graph.add<GraphicsStage>("main stage");
    main_stage.writes_to(back_buffer);
    main_stage.writes_to(depth_buffer);
    main_stage.set_clear_colour({0, 0, 0, 1}, {1.0F, 0});
    main_stage.set_on_record([&](VkCommandBuffer cmd_buf) {
        // TODO(): Nice OOP wrappers
        // TODO(): cmd_buf->draw(...);
        const VkBuffer buffers[] = {mesh_buffers[0].get_vertex_buffer()};
        const VkDeviceSize offsets[] = {0};
        vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, main_stage.pipeline_layout(), 0, 1,
                                descriptors[0].get_descriptor_sets_data(), 0, nullptr);
        vkCmdBindVertexBuffers(cmd_buf, 0, 1, buffers, offsets);
        vkCmdDraw(cmd_buf, mesh_buffers[0].get_vertex_count(), 1, 0, 0);
    });

    for (const auto &shader : shaders) {
        main_stage.uses_shader(shader);
    }

    for (const auto &attribute_binding : OctreeVertex::get_attribute_binding_description()) {
        main_stage.add_attribute_binding(attribute_binding);
    }
    main_stage.add_descriptor_layout(descriptors[0].get_descriptor_set_layout());
    main_stage.add_vertex_binding(OctreeVertex::get_vertex_binding_description());

    m_frame_graph.compile(back_buffer, vkdevice->get_device(), command_pool->get(), vma->get_allocator(), *swapchain);
}

VkResult VulkanRenderer::create_uniform_buffers() {
    uniform_buffers.emplace_back(vkdevice->get_device(), vma->get_allocator(), std::string("matrices uniform buffer"),
                                 sizeof(UniformBufferObject));

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_synchronisation_objects() {
    assert(swapchain->get_image_count() > 0);

    spdlog::debug("Creating synchronisation objects: semaphores and fences.");
    spdlog::debug("Number of images in swapchain: {}.", swapchain->get_image_count());

    in_flight_fences.clear();
    image_available_semaphores.clear();
    rendering_finished_semaphores.clear();

    // TODO: Add method to create several fences/semaphores.

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        in_flight_fences.emplace_back(vkdevice->get_device(), "In flight fence #" + std::to_string(i), true);
        image_available_semaphores.emplace_back(vkdevice->get_device(),
                                                "Image available semaphore #" + std::to_string(i));
        rendering_finished_semaphores.emplace_back(vkdevice->get_device(),
                                                   "Rendering finished semaphore #" + std::to_string(i));
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::cleanup_swapchain() {
    spdlog::debug("Cleaning up swapchain.");

    spdlog::debug("Waiting for device to be idle.");

    vkDeviceWaitIdle(vkdevice->get_device());

    spdlog::debug("Device is idle.");

    if (multisampling_enabled) {
        msaa_target_buffer.color.reset();
        msaa_target_buffer.depth.reset();
    }

    command_buffers.clear();

    pipeline_layout.reset();

    return VK_SUCCESS;
}

VkResult VulkanRenderer::update_cameras() {

    game_camera.update(time_passed);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::recreate_swapchain() {
    assert(vkdevice->get_device());

    vkDeviceWaitIdle(vkdevice->get_device());

    // TODO: outsource cleanup_swapchain() methods!

    spdlog::debug("Querying new window size.");

    window->wait_for_focus();

    window_width = window->get_width();
    window_height = window->get_height();

    spdlog::debug("New window size: width: {}, height: {}.", window_width, window_height);

    swapchain->recreate(window_width, window_height);

    if (multisampling_enabled) {
        msaa_target_buffer.color.reset();
        msaa_target_buffer.depth.reset();
    }

    framebuffer.reset();

    vkDeviceWaitIdle(vkdevice->get_device());

    // Calculate the new aspect ratio so we can update game camera matrices.
    float aspect_ratio = window_width / static_cast<float>(window_height);

    spdlog::debug("New aspect ratio: {}.", aspect_ratio);

    vkDeviceWaitIdle(vkdevice->get_device());

    // Setup game camera.
    game_camera.type = Camera::CameraType::LOOKAT;

    game_camera.set_perspective(45.0f, (float)window_width / (float)window_height, 0.1f, 256.0f);
    game_camera.rotation_speed = 0.25f;
    game_camera.movement_speed = 0.1f;
    game_camera.set_position({0.0f, 0.0f, 5.0f});
    game_camera.set_rotation({0.0f, 0.0f, 0.0f});

    vkDeviceWaitIdle(vkdevice->get_device());

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

VkResult VulkanRenderer::calculate_memory_budget() {
    VmaStats memory_stats;

    spdlog::debug(
        "------------------------------------------------------------------------------------------------------------");
    spdlog::debug("Calculating memory statistics before shutdown.");

    // Use Vulkan memory allocator's statistics.
    vmaCalculateStats(vma->get_allocator(), &memory_stats);

    spdlog::debug("VMA heap:");

    spdlog::debug("Number of `VkDeviceMemory` Vulkan memory blocks allocated: {}", memory_stats.memoryHeap->blockCount);
    spdlog::debug("Number of #VmaAllocation allocation objects allocated: {}",
                  memory_stats.memoryHeap->allocationCount);
    spdlog::debug("Number of free ranges of memory between allocations: {}", memory_stats.memoryHeap->unusedRangeCount);
    spdlog::debug("Total number of bytes occupied by all allocations: {}", memory_stats.memoryHeap->usedBytes);
    spdlog::debug("Total number of bytes occupied by unused ranges: {}", memory_stats.memoryHeap->unusedBytes);
    spdlog::debug("memory_stats.memoryHeap->allocationSizeMin: {}", memory_stats.memoryHeap->allocationSizeMin);
    spdlog::debug("memory_stats.memoryHeap->allocationSizeAvg: {}", memory_stats.memoryHeap->allocationSizeAvg);
    spdlog::debug("memory_stats.memoryHeap->allocationSizeMax: {}", memory_stats.memoryHeap->allocationSizeMax);
    spdlog::debug("memory_stats.memoryHeap->unusedRangeSizeMin: {}", memory_stats.memoryHeap->unusedRangeSizeMin);
    spdlog::debug("memory_stats.memoryHeap->unusedRangeSizeAvg: {}", memory_stats.memoryHeap->unusedRangeSizeAvg);
    spdlog::debug("memory_stats.memoryHeap->unusedRangeSizeMax: {}", memory_stats.memoryHeap->unusedRangeSizeMax);

    spdlog::debug("VMA memory type:");

    spdlog::debug("Number of `VkDeviceMemory` Vulkan memory blocks allocated: {}", memory_stats.memoryType->blockCount);
    spdlog::debug("Number of #VmaAllocation allocation objects allocated: {}",
                  memory_stats.memoryType->allocationCount);
    spdlog::debug("Number of free ranges of memory between allocations: {}", memory_stats.memoryType->unusedRangeCount);
    spdlog::debug("Total number of bytes occupied by all allocations: {}", memory_stats.memoryType->usedBytes);
    spdlog::debug("Total number of bytes occupied by unused ranges: {}", memory_stats.memoryType->unusedBytes);
    spdlog::debug("memory_stats.memoryType->allocationSizeMin: {}", memory_stats.memoryType->allocationSizeMin);
    spdlog::debug("memory_stats.memoryType->allocationSizeAvg: {}", memory_stats.memoryType->allocationSizeAvg);
    spdlog::debug("memory_stats.memoryType->allocationSizeMax: {}", memory_stats.memoryType->allocationSizeMax);
    spdlog::debug("memory_stats.memoryType->unusedRangeSizeMin: {}", memory_stats.memoryType->unusedRangeSizeMin);
    spdlog::debug("memory_stats.memoryType->unusedRangeSizeAvg: {}", memory_stats.memoryType->unusedRangeSizeAvg);
    spdlog::debug("memory_stats.memoryType->unusedRangeSizeMax: {}", memory_stats.memoryType->unusedRangeSizeMax);

    spdlog::debug("VMA total:");

    spdlog::debug("Number of `VkDeviceMemory` Vulkan memory blocks allocated: {}", memory_stats.total.blockCount);
    spdlog::debug("Number of #VmaAllocation allocation objects allocated: {}", memory_stats.total.allocationCount);
    spdlog::debug("Number of free ranges of memory between allocations: {}", memory_stats.total.unusedRangeCount);
    spdlog::debug("Total number of bytes occupied by all allocations: {}", memory_stats.total.usedBytes);
    spdlog::debug("Total number of bytes occupied by unused ranges: {}", memory_stats.total.unusedBytes);
    spdlog::debug("memory_stats.total.allocationSizeMin: {}", memory_stats.total.allocationSizeMin);
    spdlog::debug("memory_stats.total.allocationSizeAvg: {}", memory_stats.total.allocationSizeAvg);
    spdlog::debug("memory_stats.total.allocationSizeMax: {}", memory_stats.total.allocationSizeMax);
    spdlog::debug("memory_stats.total.unusedRangeSizeMin: {}", memory_stats.total.unusedRangeSizeMin);
    spdlog::debug("memory_stats.total.unusedRangeSizeAvg: {}", memory_stats.total.unusedRangeSizeAvg);
    spdlog::debug("memory_stats.total.unusedRangeSizeMax: {}", memory_stats.total.unusedRangeSizeMax);

    char *vma_stats_string = nullptr;

    vmaBuildStatsString(vma->get_allocator(), &vma_stats_string, true);

    std::ofstream vma_memory_dump;

    std::string memory_dump_file_name = "vma-dumps/inexor_VMA_dump_" + std::to_string(vma_dump_index) + ".json";

    vma_memory_dump.open(memory_dump_file_name, std::ios::out);

    vma_memory_dump.write(vma_stats_string, strlen(vma_stats_string));

    vma_memory_dump.close();

    vma_dump_index++;

    vmaFreeStatsString(vma->get_allocator(), vma_stats_string);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::shutdown_vulkan() {
    // It is important to destroy the objects in reversal of the order of creation.
    spdlog::debug(
        "------------------------------------------------------------------------------------------------------------");
    spdlog::debug("Shutting down Vulkan API.");

    cleanup_swapchain();

    // @todo: (yeetari) Remove once this class is RAII-ified.
    shaders.clear();
    textures.clear();
    uniform_buffers.clear();
    mesh_buffers.clear();
    descriptors.clear();

    image_available_semaphores.clear();
    rendering_finished_semaphores.clear();

    in_flight_fences.clear();

    vma.reset();

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

    in_flight_fences.clear();
    image_available_semaphores.clear();
    rendering_finished_semaphores.clear();

    vkinstance.reset();

    return VK_SUCCESS;
}

} // namespace inexor::vulkan_renderer
