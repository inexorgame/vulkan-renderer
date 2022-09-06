#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <vulkan/vulkan_core.h>

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
VkMemoryBarrier make_info() {
    return {.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER};
}

template <>
VkPipelineColorBlendStateCreateInfo make_info() {
    VkPipelineColorBlendStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    return ret;
}

template <>
VkPipelineDepthStencilStateCreateInfo make_info() {
    VkPipelineDepthStencilStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    return ret;
}

template <>
VkPipelineInputAssemblyStateCreateInfo make_info() {
    VkPipelineInputAssemblyStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    return ret;
}

template <>
VkPipelineLayoutCreateInfo make_info() {
    VkPipelineLayoutCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    return ret;
}

template <>
VkPipelineMultisampleStateCreateInfo make_info() {
    VkPipelineMultisampleStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    return ret;
}

template <>
VkPipelineRasterizationStateCreateInfo make_info() {
    VkPipelineRasterizationStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    return ret;
}

template <>
VkPipelineShaderStageCreateInfo make_info() {
    VkPipelineShaderStageCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    return ret;
}

template <>
VkPipelineVertexInputStateCreateInfo make_info() {
    VkPipelineVertexInputStateCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
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

template <>
VkRenderPassCreateInfo make_info() {
    VkRenderPassCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
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

} // namespace inexor::vulkan_renderer::wrapper
