#include "inexor/vulkan-renderer/renderer.hpp"

#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/octree_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <fstream>

namespace inexor::vulkan_renderer {

VkResult VulkanRenderer::create_depth_buffer() {

    const std::vector<VkFormat> supported_formats = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,
                                                     VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
                                                     VK_FORMAT_D16_UNORM};

    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags format = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    auto depth_buffer_format_candidate = settings_decision_maker->find_depth_buffer_format(
        vkdevice->get_physical_device(), supported_formats, tiling, format);

    if (!depth_buffer_format_candidate) {
        throw std::runtime_error("Error: Could not find appropriate image format for depth buffer!");
    }

    // Create depth buffer image and image view.
    depth_buffer = std::make_unique<wrapper::Image>(
        vkdevice->get_device(), vkdevice->get_physical_device(), vma->get_allocator(),
        depth_buffer_format_candidate.value(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_SAMPLE_COUNT_1_BIT, "Depth buffer", swapchain->get_extent());

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_command_buffers() {
    assert(vkdevice->get_device());
    assert(swapchain->get_image_count() > 0);

    spdlog::debug("Allocating command buffers.");
    spdlog::debug("Number of images in swapchain: {}.", swapchain->get_image_count());

    for (std::size_t i = 0; i < swapchain->get_image_count(); i++) {
        command_buffers.emplace_back(vkdevice->get_device(), command_pool->get());
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::record_command_buffers() {
    assert(window->get_width() > 0);
    assert(window->get_height() > 0);

    spdlog::debug("Recording command buffers.");

    VkCommandBufferBeginInfo command_buffer_bi = {};
    command_buffer_bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_bi.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    command_buffer_bi.pInheritanceInfo = nullptr;

    // TODO: Setup clear colors by TOML configuration file.
    std::array<VkClearValue, 3> clear_values;

    // Note that the order of clear_values must be identical to the order of the attachments.
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    VkExtent2D render_area;
    render_area.width = window->get_width();
    render_area.height = window->get_height();

    VkRenderPassBeginInfo render_pass_bi = {};
    render_pass_bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_bi.renderPass = renderpass->get();
    render_pass_bi.renderArea.offset = {0, 0};
    render_pass_bi.renderArea.extent = render_area;
    render_pass_bi.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
    render_pass_bi.pClearValues = clear_values.data();

    VkViewport viewport{};

    viewport.width = static_cast<float>(window->get_width());
    viewport.height = static_cast<float>(window->get_height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};

    scissor.extent.width = window->get_width();
    scissor.extent.height = window->get_height();

    for (std::size_t i = 0; i < swapchain->get_image_count(); i++) {
        spdlog::debug("Recording command buffer #{}.", i);

        VkCommandBuffer current_command_buffer = command_buffers[i].get();

        // TODO: Start debug marker region.

        VkResult result = vkBeginCommandBuffer(current_command_buffer, &command_buffer_bi);
        if (VK_SUCCESS != result)
            return result;

        // Update only the necessary parts of VkRenderPassBeginInfo.
        render_pass_bi.framebuffer = framebuffers[i].get();

        vkCmdBeginRenderPass(current_command_buffer, &render_pass_bi, VK_SUBPASS_CONTENTS_INLINE);

        // ----------------------------------------------------------------------------------------------------------------
        // Begin of render pass.
        {
            vkCmdSetViewport(current_command_buffer, 0, 1, &viewport);

            vkCmdSetScissor(current_command_buffer, 0, 1, &scissor);

            // TODO: Render skybox!

            vkCmdBindPipeline(current_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->get());

            vkCmdBindDescriptorSets(current_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->get(), 0,
                                    1, descriptors[0].get_descriptor_sets_data(), 0, nullptr);

            VkBuffer vertexBuffers[] = {mesh_buffers[0].get_vertex_buffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(current_command_buffer, 0, 1, vertexBuffers, offsets);

            vkCmdDraw(current_command_buffer, mesh_buffers[0].get_vertex_count(), 1, 0, 0);

            // TODO: This does not specify the order of rendering!
            // gltf_model_manager->render_all_models(command_buffers[i], pipeline_layout, i);

            // TODO: Draw imgui user interface.
        }
        // End of render pass.
        // ----------------------------------------------------------------------------------------------------------------

        vkCmdEndRenderPass(current_command_buffer);

        result = vkEndCommandBuffer(current_command_buffer);
        if (VK_SUCCESS != result)
            return result;

        // TODO: End debug marker region
    }

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

    framebuffers.clear();

    depth_buffer.reset();

    depth_stencil.reset();

    command_buffers.clear();

    graphics_pipeline.reset();

    pipeline_layout.reset();

    renderpass.reset();

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

    depth_buffer.reset();

    depth_stencil.reset();

    framebuffers.clear();

    VkResult result = create_depth_buffer();
    vulkan_error_check(result);

    result = create_frame_buffers();
    vulkan_error_check(result);

    vkDeviceWaitIdle(vkdevice->get_device());

    // Calculate the new aspect ratio so we can update game camera matrices.
    float aspect_ratio = window_width / static_cast<float>(window_height);

    spdlog::debug("New aspect ratio: {}.", aspect_ratio);

    vkDeviceWaitIdle(vkdevice->get_device());

    result = record_command_buffers();
    vulkan_error_check(result);

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

VkResult VulkanRenderer::create_pipeline() {
    assert(vkdevice->get_device());

    spdlog::debug("Creating graphics pipeline.");

    shader_stages.clear();

    assert(!shaders.empty());

    spdlog::debug("Setting up shader stages.");

    // Loop through all shaders in Vulkan shader manager's list and add them to the setup.
    for (const auto &shader : shaders) {
        VkPipelineShaderStageCreateInfo shader_stage_ci = {};
        shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_ci.stage = shader.get_type();
        shader_stage_ci.module = shader.get_module();
        shader_stage_ci.pSpecializationInfo = nullptr;
        shader_stage_ci.pName = shader.get_entry_point().c_str();

        shader_stages.push_back(shader_stage_ci);
    }

    std::vector<VkAttachmentDescription> attachments;

    VkSubpassDescription subpass_description = {};

    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments.resize(2);

    // Color attachment.
    attachments[0].format = swapchain->get_image_format();
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Depth attachment.
    attachments[1].format = depth_buffer->get_image_format();
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_reference;
    subpass_description.pDepthStencilAttachment = &depth_reference;
    subpass_description.inputAttachmentCount = 0;
    subpass_description.pInputAttachments = nullptr;
    subpass_description.preserveAttachmentCount = 0;
    subpass_description.pPreserveAttachments = nullptr;
    subpass_description.pResolveAttachments = nullptr;

    std::vector<VkSubpassDependency> dependencies(2);

    // Subpass dependencies for layout transitions.
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1] = dependencies[0];

    renderpass = std::make_unique<wrapper::RenderPass>(vkdevice->get_device(), attachments, dependencies,
                                                       subpass_description, "Default renderpass");

    const std::vector<VkDescriptorSetLayout> set_layouts = {descriptors[0].get_descriptor_set_layout()};

    pipeline_layout =
        std::make_unique<wrapper::PipelineLayout>(vkdevice->get_device(), set_layouts, "Default pipeline layout");

    const auto vertex_binding_desc = OctreeVertex::get_vertex_binding_description();
    const auto attribute_binding_desc = OctreeVertex::get_attribute_binding_description();

    graphics_pipeline = std::make_unique<wrapper::GraphicsPipeline>(
        vkdevice->get_device(), pipeline_layout->get(), renderpass->get(), shader_stages, vertex_binding_desc,
        attribute_binding_desc, window->get_width(), window->get_height(), "Default graphics pipeline");

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_frame_buffers() {
    assert(vkdevice->get_device());
    assert(window->get_width() > 0);
    assert(window->get_height() > 0);
    assert(swapchain->get_image_count() > 0);

    // Create depth stencil buffer.
    depth_stencil = std::make_unique<wrapper::Image>(
        vkdevice->get_device(), vkdevice->get_physical_device(), vma->get_allocator(), depth_buffer->get_image_format(),
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_SAMPLE_COUNT_1_BIT, "Depth stencil image",
        swapchain->get_extent());

    std::vector<VkImageView> attachments(2, nullptr);
    attachments[1] = depth_stencil->get_image_view();

    // Create framebuffers
    for (std::uint32_t i = 0; i < swapchain->get_image_count(); i++) {
        framebuffers.emplace_back(vkdevice->get_device(), swapchain->get_image_view(i), depth_stencil->get_image_view(),
                                  renderpass->get(), *swapchain);
    }

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
