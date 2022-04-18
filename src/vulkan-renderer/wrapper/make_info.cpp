#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include "inexor/vulkan-renderer/wrapper/shader.hpp"

namespace inexor::vulkan_renderer::wrapper {

template <>
VkApplicationInfo make_info() {
    VkApplicationInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    return ret;
}

template <>
VkBufferCreateInfo make_info() {
    VkBufferCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    return ret;
}

template <>
VkCommandBufferAllocateInfo make_info() {
    VkCommandBufferAllocateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    return ret;
}

template <>
VkCommandBufferBeginInfo make_info() {
    VkCommandBufferBeginInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    return ret;
}

template <>
VkCommandPoolCreateInfo make_info() {
    VkCommandPoolCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    return ret;
}

template <>
VkDebugMarkerMarkerInfoEXT make_info() {
    VkDebugMarkerMarkerInfoEXT ret{};
    ret.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
    return ret;
}

template <>
VkDebugMarkerObjectNameInfoEXT make_info() {
    VkDebugMarkerObjectNameInfoEXT ret{};
    ret.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
    return ret;
}

template <>
VkDebugMarkerObjectTagInfoEXT make_info() {
    VkDebugMarkerObjectTagInfoEXT ret{};
    ret.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
    return ret;
}

template <>
VkDebugReportCallbackCreateInfoEXT make_info() {
    VkDebugReportCallbackCreateInfoEXT ret{};
    ret.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    return ret;
}

template <>
VkDescriptorPoolCreateInfo make_info() {
    VkDescriptorPoolCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    return ret;
}

template <>
VkDescriptorSetAllocateInfo make_info() {
    VkDescriptorSetAllocateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    return ret;
}

template <>
VkDescriptorSetLayoutCreateInfo make_info() {
    VkDescriptorSetLayoutCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    return ret;
}

template <>
VkDeviceCreateInfo make_info() {
    VkDeviceCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    return ret;
}

template <>
VkDeviceQueueCreateInfo make_info() {
    VkDeviceQueueCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    return ret;
}

template <>
VkFenceCreateInfo make_info() {
    VkFenceCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    return ret;
}

template <>
VkFramebufferCreateInfo make_info() {
    VkFramebufferCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    return ret;
}

template <>
VkGraphicsPipelineCreateInfo make_info() {
    VkGraphicsPipelineCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    return ret;
}

VkGraphicsPipelineCreateInfo make_info(const VkPipelineLayout pipeline_layout, const VkRenderPass renderpass,
                                       const std::vector<VkPipelineShaderStageCreateInfo> &stages,
                                       const VkPipelineVertexInputStateCreateInfo *vertex_input_state,
                                       const VkPipelineInputAssemblyStateCreateInfo *input_assembly_state,
                                       const VkPipelineViewportStateCreateInfo *viewport_state,
                                       const VkPipelineRasterizationStateCreateInfo *rasterization_state,
                                       const VkPipelineMultisampleStateCreateInfo *multisample_state,
                                       const VkPipelineDepthStencilStateCreateInfo *depth_stencil_state,
                                       const VkPipelineColorBlendStateCreateInfo *color_blend_state,
                                       const VkPipelineDynamicStateCreateInfo *dynamic_state) {

    assert(!stages.empty());
    assert(vertex_input_state);
    assert(input_assembly_state);
    assert(viewport_state);
    assert(rasterization_state);
    assert(multisample_state);
    assert(depth_stencil_state);
    assert(color_blend_state);
    assert(dynamic_state);

    VkGraphicsPipelineCreateInfo ret{};

    ret.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ret.layout = pipeline_layout;
    ret.renderPass = renderpass;
    ret.stageCount = static_cast<std::uint32_t>(stages.size());
    ret.pStages = stages.data();
    ret.pVertexInputState = vertex_input_state;
    ret.pInputAssemblyState = input_assembly_state;
    ret.pViewportState = viewport_state;
    ret.pRasterizationState = rasterization_state;
    ret.pMultisampleState = multisample_state;
    ret.pDepthStencilState = depth_stencil_state;
    ret.pColorBlendState = color_blend_state;
    ret.pDynamicState = dynamic_state;

    return ret;
}

template <typename PipelineWrapperType, typename RenderpassWrapperType, typename ShaderWrapperType>
VkGraphicsPipelineCreateInfo make_info(const PipelineWrapperType &pipeline_wrapper,
                                       const RenderpassWrapperType &renderpass_wrapper,
                                       const ShaderWrapperType &shader_loader, 
                                       const VkPipelineVertexInputStateCreateInfo &vertex_input_sci,
                                       const VkPipelineInputAssemblyStateCreateInfo &input_assembly_sci,
                                       const VkPipelineViewportStateCreateInfo &viewport_sci,
                                       const VkPipelineRasterizationStateCreateInfo *&rasterization_sci,
                                       const VkPipelineMultisampleStateCreateInfo &multisample_sci,
                                       const VkPipelineDepthStencilStateCreateInfo &depth_stencil_sci,
                                       const VkPipelineColorBlendStateCreateInfo &color_blend_sci,
                                       const VkPipelineDynamicStateCreateInfo &dynamic_state_ci) {
    return make_info(pipeline_wrapper.pipeline_layout(), renderpass_wrapper.renderpass(),
                     shader_loader.shader_stage_create_infos(), &vertex_input_sci, &input_assembly_sci, &viewport_sci,
                     &rasterization_sci, &multisample_sci, &depth_stencil_sci, &color_blend_sci, &dynamic_state_ci);
}

template <>
VkImageCreateInfo make_info() {
    VkImageCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    return ret;
}

template <>
VkImageMemoryBarrier make_info() {
    VkImageMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    return ret;
}

template <>
VkImageViewCreateInfo make_info() {
    VkImageViewCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    return ret;
}

template <>
VkInstanceCreateInfo make_info() {
    VkInstanceCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    return ret;
}

template <>
VkPipelineColorBlendStateCreateInfo make_info() {
    VkPipelineColorBlendStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    return ret;
}

VkPipelineColorBlendStateCreateInfo make_info(const std::vector<VkPipelineColorBlendAttachmentState> &attachments) {
    VkPipelineColorBlendStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ret.pAttachments = attachments.data();
    ret.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    return ret;
}

template <>
VkPipelineDepthStencilStateCreateInfo make_info() {
    VkPipelineDepthStencilStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ret.depthTestEnable = VK_FALSE;
    ret.depthWriteEnable = VK_FALSE;
    ret.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ret.front = ret.back;
    ret.back.compareOp = VK_COMPARE_OP_ALWAYS;
    return ret;
}

template <>
VkPipelineDynamicStateCreateInfo make_info() {
    VkPipelineDynamicStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    return ret;
}

VkPipelineDynamicStateCreateInfo make_info(const std::vector<VkDynamicState> &dynamic_states) {
    VkPipelineDynamicStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    ret.dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size());
    ret.pDynamicStates = dynamic_states.data();
    return ret;
}

template <>
VkPipelineInputAssemblyStateCreateInfo make_info() {
    VkPipelineInputAssemblyStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ret.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    ret.primitiveRestartEnable = VK_FALSE;
    return ret;
}

template <>
VkPipelineLayoutCreateInfo make_info() {
    VkPipelineLayoutCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    return ret;
}

VkPipelineLayoutCreateInfo make_info(const std::vector<VkDescriptorSetLayout> &set_layouts,
                                     const std::vector<VkPushConstantRange> &push_constant_ranges) {

    assert(!set_layouts.empty());
    // Note that push_constant_ranges is allowed to be empty

    VkPipelineLayoutCreateInfo ret{};

    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    ret.pushConstantRangeCount = static_cast<std::uint32_t>(push_constant_ranges.size());
    ret.pPushConstantRanges = push_constant_ranges.data();
    ret.setLayoutCount = static_cast<std::uint32_t>(set_layouts.size());
    ret.pSetLayouts = set_layouts.data();

    return ret;
}

template <>
VkPipelineMultisampleStateCreateInfo make_info() {
    VkPipelineMultisampleStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ret.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    return ret;
}

template <>
VkPipelineRasterizationStateCreateInfo make_info() {
    VkPipelineRasterizationStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    ret.polygonMode = VK_POLYGON_MODE_FILL;
    ret.cullMode = VK_CULL_MODE_NONE;
    ret.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    ret.lineWidth = 1.0f;
    return ret;
}

template <>
VkPipelineShaderStageCreateInfo make_info() {
    VkPipelineShaderStageCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    return ret;
}

VkPipelineShaderStageCreateInfo make_info(const wrapper::Shader &shader) {
    VkPipelineShaderStageCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ret.module = shader.module();
    ret.stage = shader.type();
    ret.pName = shader.entry_point().c_str();
    return ret;
}

template <>
VkPipelineVertexInputStateCreateInfo make_info() {
    VkPipelineVertexInputStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    return ret;
}

VkPipelineVertexInputStateCreateInfo
make_info(const std::vector<VkVertexInputBindingDescription> &vertex_input_binding_descriptions,
          const std::vector<VkVertexInputAttributeDescription> &vertex_input_attribute_descriptions) {

    assert(!vertex_input_binding_descriptions.empty());
    assert(!vertex_input_attribute_descriptions.empty());

    VkPipelineVertexInputStateCreateInfo ret{};

    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    ret.vertexBindingDescriptionCount = static_cast<std::uint32_t>(vertex_input_binding_descriptions.size());
    ret.pVertexBindingDescriptions = vertex_input_binding_descriptions.data();
    ret.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertex_input_attribute_descriptions.size());
    ret.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();

    return ret;
}

template <>
VkPipelineViewportStateCreateInfo make_info() {
    VkPipelineViewportStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    return ret;
}

template <>
VkPresentInfoKHR make_info() {
    VkPresentInfoKHR ret{};
    ret.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    return ret;
}

template <>
VkRenderPassBeginInfo make_info() {
    VkRenderPassBeginInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    return ret;
}

VkRenderPassBeginInfo make_info(const VkRenderPass renderpass, const VkFramebuffer framebuffer,
                                const VkRect2D &render_area, const std::vector<VkClearValue> &clear_values) {
    VkRenderPassBeginInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    ret.renderPass = renderpass;
    ret.framebuffer = framebuffer;
    ret.renderArea = render_area;
    ret.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
    ret.pClearValues = clear_values.data();

    return ret;
}

template <>
VkRenderPassCreateInfo make_info() {
    VkRenderPassCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    return ret;
}

VkRenderPassCreateInfo make_info(const std::vector<VkAttachmentDescription> &attachments,
                                 const std::vector<VkSubpassDescription> &subpasses,
                                 const std::vector<VkSubpassDependency> &dependencies) {

    // TODO: Does this make sense? Some of these values could be empty in a valid use case!
    assert(!attachments.empty());
    assert(!subpasses.empty());
    assert(!dependencies.empty());

    VkRenderPassCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    ret.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    ret.pAttachments = attachments.data();
    ret.subpassCount = static_cast<std::uint32_t>(subpasses.size());
    ret.pSubpasses = subpasses.data();
    ret.dependencyCount = static_cast<std::uint32_t>(dependencies.size());
    ret.pDependencies = dependencies.data();

    return ret;
}

template <>
VkSamplerCreateInfo make_info() {
    VkSamplerCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    return ret;
}

template <>
VkSemaphoreCreateInfo make_info() {
    VkSemaphoreCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    return ret;
}

template <>
VkShaderModuleCreateInfo make_info() {
    VkShaderModuleCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    return ret;
}

template <>
VkSubmitInfo make_info() {
    VkSubmitInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    return ret;
}

template <>
VkSwapchainCreateInfoKHR make_info() {
    VkSwapchainCreateInfoKHR ret{};
    ret.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    return ret;
}

template <>
VkWriteDescriptorSet make_info() {
    VkWriteDescriptorSet ret{};
    ret.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    return ret;
}

} // namespace inexor::vulkan_renderer::wrapper
