#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer::wrapper {

template <>
VkApplicationInfo make_info(VkApplicationInfo info) {
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    return info;
}

template <>
VkBufferCreateInfo make_info(VkBufferCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    return info;
}

template <>
VkCommandBufferAllocateInfo make_info(VkCommandBufferAllocateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    return info;
}

template <>
VkCommandBufferBeginInfo make_info(VkCommandBufferBeginInfo info) {
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    return info;
}

template <>
VkCommandPoolCreateInfo make_info(VkCommandPoolCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    return info;
}

template <>
VkDebugMarkerMarkerInfoEXT make_info(VkDebugMarkerMarkerInfoEXT info) {
    info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
    return info;
}

template <>
VkDebugMarkerObjectNameInfoEXT make_info(VkDebugMarkerObjectNameInfoEXT info) {
    info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
    return info;
}

template <>
VkDebugMarkerObjectTagInfoEXT make_info(VkDebugMarkerObjectTagInfoEXT info) {
    info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
    return info;
}

template <>
VkDebugReportCallbackCreateInfoEXT make_info(VkDebugReportCallbackCreateInfoEXT info) {
    info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    return info;
}

template <>
VkDebugUtilsMessengerCreateInfoEXT make_info(VkDebugUtilsMessengerCreateInfoEXT info) {
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    return info;
}

template <>
VkDescriptorPoolCreateInfo make_info(VkDescriptorPoolCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    return info;
}

template <>
VkDescriptorSetAllocateInfo make_info(VkDescriptorSetAllocateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    return info;
}

template <>
VkDescriptorSetLayoutCreateInfo make_info(VkDescriptorSetLayoutCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    return info;
}

template <>
VkDeviceCreateInfo make_info(VkDeviceCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    return info;
}

template <>
VkDeviceQueueCreateInfo make_info(VkDeviceQueueCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    return info;
}

template <>
VkFenceCreateInfo make_info(VkFenceCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    return info;
}

template <>
VkFramebufferCreateInfo make_info(VkFramebufferCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    return info;
}

template <>
VkGraphicsPipelineCreateInfo make_info(VkGraphicsPipelineCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    return info;
}

template <>
VkImageCreateInfo make_info(VkImageCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    return info;
}

template <>
VkImageMemoryBarrier make_info(VkImageMemoryBarrier info) {
    info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    return info;
}

template <>
VkImageViewCreateInfo make_info(VkImageViewCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    return info;
}

template <>
VkInstanceCreateInfo make_info(VkInstanceCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    return info;
}

template <>
VkMemoryBarrier make_info(VkMemoryBarrier info) {
    info.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    return info;
}

template <>
VkPipelineColorBlendStateCreateInfo make_info(VkPipelineColorBlendStateCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    return info;
}

template <>
VkPipelineDepthStencilStateCreateInfo make_info(VkPipelineDepthStencilStateCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    return info;
}

template <>
VkPipelineDynamicStateCreateInfo make_info(VkPipelineDynamicStateCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    return info;
}

template <>
VkPipelineInputAssemblyStateCreateInfo make_info(VkPipelineInputAssemblyStateCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    return info;
}

template <>
VkPipelineLayoutCreateInfo make_info(VkPipelineLayoutCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    return info;
}

template <>
VkPipelineMultisampleStateCreateInfo make_info(VkPipelineMultisampleStateCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    return info;
}

template <>
VkPipelineRasterizationStateCreateInfo make_info(VkPipelineRasterizationStateCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    return info;
}

template <>
VkPipelineShaderStageCreateInfo make_info(VkPipelineShaderStageCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    return info;
}

template <>
VkPipelineTessellationStateCreateInfo make_info(VkPipelineTessellationStateCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    return info;
}

template <>
VkPipelineVertexInputStateCreateInfo make_info(VkPipelineVertexInputStateCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    return info;
}

template <>
VkPipelineViewportStateCreateInfo make_info(VkPipelineViewportStateCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    return info;
}

template <>
VkPresentInfoKHR make_info(VkPresentInfoKHR info) {
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    return info;
}

template <>
VkRenderPassBeginInfo make_info(VkRenderPassBeginInfo info) {
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    return info;
}

template <>
VkRenderPassCreateInfo make_info(VkRenderPassCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    return info;
}

template <>
VkSamplerCreateInfo make_info(VkSamplerCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    return info;
}

template <>
VkSemaphoreCreateInfo make_info(VkSemaphoreCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    return info;
}

template <>
VkShaderModuleCreateInfo make_info(VkShaderModuleCreateInfo info) {
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    return info;
}

template <>
VkSubmitInfo make_info(VkSubmitInfo info) {
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    return info;
}

template <>
VkSwapchainCreateInfoKHR make_info(VkSwapchainCreateInfoKHR info) {
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    return info;
}

template <>
VkWriteDescriptorSet make_info(VkWriteDescriptorSet info) {
    info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    return info;
}

} // namespace inexor::vulkan_renderer::wrapper
