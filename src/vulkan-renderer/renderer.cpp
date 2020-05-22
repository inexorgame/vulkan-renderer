#include "inexor/vulkan-renderer/renderer.hpp"

#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/octree_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"

#include <spdlog/spdlog.h>

#include <fstream>

namespace inexor::vulkan_renderer {

VkResult VulkanRenderer::initialise_debug_marker_manager(const bool enable_debug_markers) {
    assert(vkdevice->get_device());
    assert(vkdevice->get_physical_device());

    spdlog::debug("Initialising debug marker manager.");

#ifndef NDEBUG
    if (!enable_debug_markers) {
        spdlog::warn("Vulkan debug markers are not enabled!");
        spdlog::warn("This will be of disadvantage when debugging the application with e.g. RenderDoc.");
    }
#endif

    debug_marker_manager->init(vkdevice->get_device(), vkdevice->get_physical_device(), enable_debug_markers);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_uniform_buffers() {
    uniform_buffers.emplace_back(vkdevice->get_device(), vma->get_allocator(), std::string("matrices uniform buffer"), sizeof(UniformBufferObject));

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_depth_buffer() {

    const std::vector<VkFormat> supported_formats = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT,
                                                     VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};

    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags format = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    auto depth_buffer_format_candidate = settings_decision_maker->find_depth_buffer_format(vkdevice->get_physical_device(), supported_formats, tiling, format);

    if (!depth_buffer_format_candidate) {
        throw std::runtime_error("Error: Could not find appropriate image format for depth buffer!");
    }

    // Create depth buffer image and image view.
    depth_buffer = std::make_unique<wrapper::Image>(vkdevice->get_device(), vkdevice->get_physical_device(), vma->get_allocator(),
                                                    depth_buffer_format_candidate.value(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                    VK_IMAGE_ASPECT_DEPTH_BIT, VK_SAMPLE_COUNT_1_BIT, "Depth buffer", swapchain->get_extent());

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_command_buffers() {
    assert(vkdevice->get_device());
    assert(debug_marker_manager);
    assert(swapchain->get_image_count() > 0);

    spdlog::debug("Allocating command buffers.");
    spdlog::debug("Number of images in swapchain: {}.", swapchain->get_image_count());

    for (std::size_t i = 0; i < swapchain->get_image_count(); i++) {
        command_buffers.emplace_back(vkdevice->get_device(), command_pool->get());
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::record_command_buffers() {
    assert(debug_marker_manager);
    assert(window->get_width() > 0);
    assert(window->get_height() > 0);

    spdlog::debug("Recording command buffers.");

    VkCommandBufferBeginInfo command_buffer_begin_info = {};

    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    // TODO: Setup clear colors by TOML configuration file.
    std::array<VkClearValue, 3> clear_values;

    // Note that the order of clear_values must be identical to the order of the attachments.
    if (multisampling_enabled) {
        clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_values[1].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_values[2].depthStencil = {1.0f, 0};
    } else {
        clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_values[1].depthStencil = {1.0f, 0};
    }

    VkExtent2D render_area;
    render_area.width = window->get_width();
    render_area.height = window->get_height();

    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderArea.extent = render_area;
    render_pass_begin_info.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
    render_pass_begin_info.pClearValues = clear_values.data();

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

        // TODO: Fix debug marker regions in RenderDoc.
        // Start binding the region with Vulkan debug markers.
        debug_marker_manager->bind_region(current_command_buffer, "Beginning of rendering.", DEBUG_MARKER_GREEN);

        VkResult result = vkBeginCommandBuffer(current_command_buffer, &command_buffer_begin_info);
        if (VK_SUCCESS != result)
            return result;

        // Update only the necessary parts of VkRenderPassBeginInfo.
        render_pass_begin_info.framebuffer = frame_buffers[i];

        vkCmdBeginRenderPass(current_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        // ----------------------------------------------------------------------------------------------------------------
        // Begin of render pass.
        {
            vkCmdSetViewport(current_command_buffer, 0, 1, &viewport);

            vkCmdSetScissor(current_command_buffer, 0, 1, &scissor);

            // TODO: Render skybox!

            vkCmdBindPipeline(current_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            vkCmdBindDescriptorSets(current_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, descriptors[0].get_descriptor_sets_data(),
                                    0, nullptr);

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

        debug_marker_manager->end_region(current_command_buffer);
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_synchronisation_objects() {
    assert(swapchain->get_image_count() > 0);
    assert(debug_marker_manager);

    spdlog::debug("Creating synchronisation objects: semaphores and fences.");
    spdlog::debug("Number of images in swapchain: {}.", swapchain->get_image_count());

    in_flight_fences.clear();
    image_available_semaphores.clear();
    rendering_finished_semaphores.clear();

    // TODO: Add method to create several fences/semaphores.

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        in_flight_fences.emplace_back(vkdevice->get_device(), "In flight fence #" + std::to_string(i), true);
        image_available_semaphores.emplace_back(vkdevice->get_device(), "Image available semaphore #" + std::to_string(i));
        rendering_finished_semaphores.emplace_back(vkdevice->get_device(), "Rendering finished semaphore #" + std::to_string(i));
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::cleanup_swapchain() {
    spdlog::debug("Cleaning up swapchain.");

    spdlog::debug("Waiting for device to be idle.");

    vkDeviceWaitIdle(vkdevice->get_device());

    spdlog::debug("Device is idle.");

    spdlog::debug("Destroying frame buffer.");

    if (frame_buffers.size() > 0) {
        for (auto *frame_buffer : frame_buffers) {
            if (VK_NULL_HANDLE != frame_buffer) {
                vkDestroyFramebuffer(vkdevice->get_device(), frame_buffer, nullptr);
                frame_buffer = VK_NULL_HANDLE;
            }
        }

        frame_buffers.clear();
    }

    spdlog::debug("Destroying depth buffer image view.");

    depth_buffer.reset();

    depth_stencil.reset();

    if (multisampling_enabled) {
        msaa_target_buffer.color.reset();
        msaa_target_buffer.depth.reset();
    }

    spdlog::debug("Destroying command buffers.");

    command_buffers.clear();

    spdlog::debug("Destroying pipeline.");

    if (VK_NULL_HANDLE != pipeline) {
        vkDestroyPipeline(vkdevice->get_device(), pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying pipeline layout.");

    if (VK_NULL_HANDLE != pipeline_layout) {
        vkDestroyPipelineLayout(vkdevice->get_device(), pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying render pass.");

    if (VK_NULL_HANDLE != render_pass) {
        vkDestroyRenderPass(vkdevice->get_device(), render_pass, nullptr);
        render_pass = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying image views.");

    spdlog::debug("Destroying descriptor sets and layouts.");

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

    spdlog::debug("Destroying depth buffer image view.");

    depth_buffer.reset();

    depth_stencil.reset();

    if (multisampling_enabled) {
        msaa_target_buffer.color.reset();
        msaa_target_buffer.depth.reset();
    }

    // TODO: Create methods for destroying resources as well!
    spdlog::debug("Destroying frame buffer.");

    if (frame_buffers.size() > 0) {
        for (auto *frame_buffer : frame_buffers) {
            if (VK_NULL_HANDLE != frame_buffer) {
                vkDestroyFramebuffer(vkdevice->get_device(), frame_buffer, nullptr);
                frame_buffer = VK_NULL_HANDLE;
            }
        }

        frame_buffers.clear();
    }

    VkResult result = create_depth_buffer();
    vulkan_error_check(result);

    result = create_frame_buffers();
    vulkan_error_check(result);

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

    result = record_command_buffers();
    vulkan_error_check(result);

    vkDeviceWaitIdle(vkdevice->get_device());

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_descriptor_pool() {
    descriptors.emplace_back(vkdevice->get_device(), swapchain->get_image_count(), std::string("unnamed descriptor"));

    // Create the descriptor pool.
    descriptors[0].create_descriptor_pool({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});

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
    assert(debug_marker_manager);

    spdlog::debug("Creating graphics pipeline.");

    shader_stages.clear();

    assert(!shaders.empty());

    spdlog::debug("Setting up shader stages.");

    // Loop through all shaders in Vulkan shader manager's list and add them to the setup.
    for (const auto &shader : shaders) {
        VkPipelineShaderStageCreateInfo shader_stage_create_info = {};

        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.pNext = nullptr;
        shader_stage_create_info.flags = 0;
        shader_stage_create_info.stage = shader.get_type();
        shader_stage_create_info.module = shader.get_module();
        shader_stage_create_info.pSpecializationInfo = nullptr;
        shader_stage_create_info.pName = shader.get_entry_point().c_str();

        shader_stages.push_back(shader_stage_create_info);
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {};

    // auto vertex_binding_description = gltf_model::ModelVertex::get_vertex_binding_description();
    // auto attribute_binding_description = gltf_model::ModelVertex::get_attribute_binding_description();

    auto vertex_binding_description = OctreeVertex::get_vertex_binding_description();
    auto attribute_binding_description = OctreeVertex::get_attribute_binding_description();

    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.pNext = nullptr;
    vertex_input_create_info.flags = 0;
    vertex_input_create_info.vertexBindingDescriptionCount = 1;
    vertex_input_create_info.pVertexBindingDescriptions = &vertex_binding_description;
    vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attribute_binding_description.size());
    vertex_input_create_info.pVertexAttributeDescriptions = attribute_binding_description.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};

    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.pNext = nullptr;
    input_assembly_create_info.flags = 0;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    VkViewport view_port = {};

    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = static_cast<float>(window_width);
    view_port.height = static_cast<float>(window_height);
    view_port.minDepth = 0.0f;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor = {};

    scissor.offset = {0, 0};
    scissor.extent = {window_width, window_height};

    // TODO: Multiple viewports (and scissors) - ViewportManager and RenderSceneManager
    VkPipelineViewportStateCreateInfo pipeline_viewport_viewport_state_info = {};

    pipeline_viewport_viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipeline_viewport_viewport_state_info.pNext = nullptr;
    pipeline_viewport_viewport_state_info.flags = 0;
    pipeline_viewport_viewport_state_info.viewportCount = 1;
    pipeline_viewport_viewport_state_info.pViewports = &view_port;
    pipeline_viewport_viewport_state_info.scissorCount = 1;
    pipeline_viewport_viewport_state_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info = {};

    pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipeline_rasterization_state_create_info.pNext = nullptr;
    pipeline_rasterization_state_create_info.flags = 0;
    pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
    pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;

    // TODO: Implement -wireframe command line argument.
    // Because the pipeline in Vulkan is immutable, this guides us to record a second command line with wireframe enabled.
    pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
    pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
    pipeline_rasterization_state_create_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_create_info = {};

    multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.pNext = nullptr;
    multisample_create_info.flags = 0;
    multisample_create_info.sampleShadingEnable = VK_FALSE;
    multisample_create_info.minSampleShading = 1.0f;
    multisample_create_info.pSampleMask = nullptr;
    multisample_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_create_info.alphaToOneEnable = VK_FALSE;

    if (multisampling_enabled) {
        multisample_create_info.rasterizationSamples = multisampling_sample_count;
    } else {
        multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};

    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};

    // TODO: Do we need this yet?
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};

    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.pNext = nullptr;
    color_blend_state_create_info.flags = 0;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_NO_OP;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment;
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;

    const std::vector<VkDescriptorSetLayout> set_layouts = {descriptors[0].get_descriptor_set_layout()};

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};

    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pNext = nullptr;
    pipeline_layout_create_info.flags = 0;
    pipeline_layout_create_info.setLayoutCount = static_cast<std::uint32_t>(set_layouts.size());
    pipeline_layout_create_info.pSetLayouts = set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;

    spdlog::debug("Setting up pipeline layout.");

    VkResult result = vkCreatePipelineLayout(vkdevice->get_device(), &pipeline_layout_create_info, nullptr, &pipeline_layout);
    if (VK_SUCCESS != result)
        return result;

    // Use Vulkan debug markers to assign an appropriate name to this pipeline.
    debug_marker_manager->set_object_name(vkdevice->get_device(), (std::uint64_t)(pipeline_layout), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
                                          "Pipeline layout for core engine.");

    if (multisampling_enabled) {
        spdlog::debug("Multisampling is enabled.");

        //
        std::array<VkAttachmentDescription, 4> attachments = {};

        // Multisampled attachment that we render to
        attachments[0].format = swapchain->get_image_format();
        attachments[0].samples = multisampling_sample_count;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // This is the frame buffer attachment to where the multisampled image
        // will be resolved to and which will be presented to the swapchain.
        attachments[1].format = swapchain->get_image_format();
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Multisampled depth attachment we render to
        attachments[2].format = depth_buffer->get_image_format();
        attachments[2].samples = multisampling_sample_count;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Depth resolve attachment
        attachments[3].format = depth_buffer->get_image_format();
        attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_reference = {};

        color_reference.attachment = 0;
        color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_reference = {};

        depth_reference.attachment = 2;
        depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Resolve attachment reference for the color attachment.
        VkAttachmentReference resolve_reference = {};

        resolve_reference.attachment = 1;
        resolve_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};

        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_reference;
        subpass.pResolveAttachments = &resolve_reference;
        subpass.pDepthStencilAttachment = &depth_reference;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderpass_create_info = {};

        renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
        renderpass_create_info.pAttachments = attachments.data();
        renderpass_create_info.subpassCount = 1;
        renderpass_create_info.pSubpasses = &subpass;
        renderpass_create_info.dependencyCount = 2;
        renderpass_create_info.pDependencies = dependencies.data();

        spdlog::debug("Creating renderpass.");

        result = vkCreateRenderPass(vkdevice->get_device(), &renderpass_create_info, nullptr, &render_pass);
        vulkan_error_check(result);
    } else {
        spdlog::debug("Multisampling is disabled.");

        std::array<VkAttachmentDescription, 2> attachments = {};

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

        VkAttachmentReference color_reference = {};

        color_reference.attachment = 0;
        color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_reference = {};

        depth_reference.attachment = 1;
        depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = {};

        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &color_reference;
        subpassDescription.pDepthStencilAttachment = &depth_reference;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pResolveAttachments = nullptr;

        // Subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderpass_create_info{};

        renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
        renderpass_create_info.pAttachments = attachments.data();
        renderpass_create_info.subpassCount = 1;
        renderpass_create_info.pSubpasses = &subpassDescription;
        renderpass_create_info.dependencyCount = static_cast<std::uint32_t>(dependencies.size());
        renderpass_create_info.pDependencies = dependencies.data();

        spdlog::debug("Creating renderpass.");

        result = vkCreateRenderPass(vkdevice->get_device(), &renderpass_create_info, nullptr, &render_pass);
        vulkan_error_check(result);
    }

    // Use Vulkan debug markers to assign an appropriate name to this renderpass.
    debug_marker_manager->set_object_name(vkdevice->get_device(), (std::uint64_t)(render_pass), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                                          "Render pass for core engine.");

    // Tell Vulkan that we want to change viewport and scissor during runtime so it's a dynamic state.
    const std::vector<VkDynamicState> enabled_dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    // TODO: Wrap all this into RenderingPipelineManager instead of loading from TOML file?
    // RenderingPipelineManager could verify if all structures were filled correctly.

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};

    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.pDynamicStates = enabled_dynamic_states.data();
    dynamic_state_create_info.dynamicStateCount = static_cast<std::uint32_t>(enabled_dynamic_states.size());

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};

    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.pNext = nullptr;
    graphics_pipeline_create_info.flags = 0;
    graphics_pipeline_create_info.stageCount = static_cast<std::uint32_t>(shader_stages.size());
    graphics_pipeline_create_info.pStages = shader_stages.data();
    graphics_pipeline_create_info.pVertexInputState = &vertex_input_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    graphics_pipeline_create_info.pTessellationState = nullptr;
    graphics_pipeline_create_info.pViewportState = &pipeline_viewport_viewport_state_info;
    graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
    graphics_pipeline_create_info.pMultisampleState = &multisample_create_info;
    graphics_pipeline_create_info.pDepthStencilState = &depth_stencil;
    graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    graphics_pipeline_create_info.layout = pipeline_layout;
    graphics_pipeline_create_info.renderPass = render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = -1;

    spdlog::debug("Creating pipeline cache.");

    VkPipelineCacheCreateInfo pipeline_cache_create_info = {};

    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    /// Vulkan Spec:
    /// "Pipeline cache objects allow the result of pipeline construction to be reused between pipelines and between runs of an application.
    /// Reuse between pipelines is achieved by passing the same pipeline cache object when creating multiple related pipelines. Reuse across
    /// runs of an application is achieved by retrieving pipeline cache contents in one run of an application, saving the contents, and using
    /// them to preinitialize a pipeline cache on a subsequent run. The contents of the pipeline cache objects are managed by the implementation.
    /// Applications can manage the host memory consumed by a pipeline cache object and control the amount of data retrieved from a pipeline cache object."
    result = vkCreatePipelineCache(vkdevice->get_device(), &pipeline_cache_create_info, nullptr, &pipeline_cache);
    vulkan_error_check(result);

    spdlog::debug("Finalizing graphics pipeline.");

    // TODO: Create multiple pipelines at once?
    result = vkCreateGraphicsPipelines(vkdevice->get_device(), pipeline_cache, 1, &graphics_pipeline_create_info, nullptr, &pipeline);
    if (VK_SUCCESS != result)
        return result;

    // Use Vulkan debug markers to assign an appropriate name to this pipeline.
    debug_marker_manager->set_object_name(vkdevice->get_device(), (std::uint64_t)(pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                                          "Graphics pipeline for core engine.");

    // TODO: We could destroy shader modules here already.
    // TODO: Create alpha blend pipeline.
    // TODO: Create PBR pipeline.

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_frame_buffers() {
    assert(vkdevice->get_device());
    assert(window->get_width() > 0);
    assert(window->get_height() > 0);
    assert(debug_marker_manager);
    assert(swapchain->get_image_count() > 0);

    if (multisampling_enabled) {
        // Check if device supports requested sample count for color and depth frame buffer
        // assert((deviceProperties.limits.framebufferColorSampleCounts >= sampleCount) && (deviceProperties.limits.framebufferDepthSampleCounts >=
        // sampleCount));

        // Create color buffer for MSAA target.
        msaa_target_buffer.color =
            std::make_unique<wrapper::Image>(vkdevice->get_device(), vkdevice->get_physical_device(), vma->get_allocator(), swapchain->get_image_format(),
                                             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
                                             multisampling_sample_count, "MSAA color image", swapchain->get_extent());

        // Create depth buffer for MSAA target.
        msaa_target_buffer.depth = std::make_unique<wrapper::Image>(
            vkdevice->get_device(), vkdevice->get_physical_device(), vma->get_allocator(), depth_buffer->get_image_format(),
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
            multisampling_sample_count, "MSAA depth image", swapchain->get_extent());
    }

    // Create depth stencil buffer.
    depth_stencil = std::make_unique<wrapper::Image>(
        vkdevice->get_device(), vkdevice->get_physical_device(), vma->get_allocator(), depth_buffer->get_image_format(),
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
        VK_SAMPLE_COUNT_1_BIT, "Depth stencil image", swapchain->get_extent());

    VkImageView attachments[4];

    if (multisampling_enabled) {
        attachments[0] = msaa_target_buffer.color->get_image_view();
        attachments[2] = msaa_target_buffer.depth->get_image_view();
        attachments[3] = depth_stencil->get_image_view();
    } else {
        attachments[1] = depth_stencil->get_image_view();
    }

    VkFramebufferCreateInfo frame_buffer_create_info = {};

    frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_create_info.pNext = nullptr;
    frame_buffer_create_info.renderPass = render_pass;
    frame_buffer_create_info.attachmentCount = multisampling_enabled ? 4 : 2;
    frame_buffer_create_info.pAttachments = attachments;
    frame_buffer_create_info.width = window->get_width();
    frame_buffer_create_info.height = window->get_height();
    frame_buffer_create_info.layers = 1;

    spdlog::debug("Creating frame buffers.");
    spdlog::debug("Number of images in swapchain: {}.", swapchain->get_image_count());

    frame_buffers.clear();
    frame_buffers.resize(swapchain->get_image_count());

    // Create frame buffers for every swap chain image.
    for (std::size_t i = 0; i < swapchain->get_image_count(); i++) {
        spdlog::debug("Creating framebuffer #{}.", i);

        if (multisampling_enabled) {
            attachments[1] = swapchain->get_image_view(i);
        } else {
            attachments[0] = swapchain->get_image_view(i);
        }

        VkResult result = vkCreateFramebuffer(vkdevice->get_device(), &frame_buffer_create_info, nullptr, &frame_buffers[i]);
        vulkan_error_check(result);

        std::string frame_buffer_name = "Frame buffer #" + std::to_string(i) + ".";

        // Use Vulkan debug markers to assign an appropriate name to this frame buffer.
        debug_marker_manager->set_object_name(vkdevice->get_device(), (std::uint64_t)(frame_buffers[i]), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT,
                                              frame_buffer_name.c_str());
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::calculate_memory_budget() {
    VmaStats memory_stats;

    spdlog::debug("------------------------------------------------------------------------------------------------------------");
    spdlog::debug("Calculating memory statistics before shutdown.");

    // Use Vulkan memory allocator's statistics.
    vmaCalculateStats(vma->get_allocator(), &memory_stats);

    spdlog::debug("VMA heap:");

    spdlog::debug("Number of `VkDeviceMemory` Vulkan memory blocks allocated: {}", memory_stats.memoryHeap->blockCount);
    spdlog::debug("Number of #VmaAllocation allocation objects allocated: {}", memory_stats.memoryHeap->allocationCount);
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
    spdlog::debug("Number of #VmaAllocation allocation objects allocated: {}", memory_stats.memoryType->allocationCount);
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
    spdlog::debug("------------------------------------------------------------------------------------------------------------");
    spdlog::debug("Shutting down Vulkan API.");

    cleanup_swapchain();

    // @todo: (yeetari) Remove once this class is RAII-ified.
    shaders.clear();
    textures.clear();
    uniform_buffers.clear();
    mesh_buffers.clear();
    descriptors.clear();

    spdlog::debug("Destroying semaphores.");
    image_available_semaphores.clear();
    rendering_finished_semaphores.clear();

    spdlog::debug("Destroying fences.");
    in_flight_fences.clear();

    vma.reset();

    spdlog::debug("Destroying Vulkan pipeline cache.");

    if (VK_NULL_HANDLE != pipeline_cache) {
        vkDestroyPipelineCache(vkdevice->get_device(), pipeline_cache, nullptr);
        pipeline_cache = VK_NULL_HANDLE;
    }

    // @todo: (Hanni) Remove them once this class is RAII-ified.
    command_pool.reset();
    swapchain.reset();
    surface.reset();
    vkdevice.reset();

    // Destroy Vulkan debug callback.
    if (debug_report_callback_initialised) {
        // We have to explicitly load this function.
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
            reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(vkinstance->get_instance(), "vkDestroyDebugReportCallbackEXT"));

        if (nullptr != vkDestroyDebugReportCallbackEXT) {
            vkDestroyDebugReportCallbackEXT(vkinstance->get_instance(), debug_report_callback, nullptr);
            debug_report_callback_initialised = false;
        }
    }

    spdlog::debug("Shutdown finished.");
    spdlog::debug("------------------------------------------------------------------------------------------------------------");

    in_flight_fences.clear();
    image_available_semaphores.clear();
    rendering_finished_semaphores.clear();

    vkinstance.reset();

    return VK_SUCCESS;
}

} // namespace inexor::vulkan_renderer
