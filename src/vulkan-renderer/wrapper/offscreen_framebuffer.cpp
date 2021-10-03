#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"

namespace inexor::vulkan_renderer::wrapper {

OffscreenFramebuffer::OffscreenFramebuffer(const wrapper::Device &device, const VkFormat format,
                                           const std::uint32_t width, const std::uint32_t height,
                                           const VkRenderPass renderpass, std::string name) {

    /*
    const Device &device, const VkImageCreateFlags flags, const VkImageType image_type, const VkFormat format,
const std::uint32_t width, const std::uint32_t height, const std::uint32_t miplevel_count,
const std::uint32_t array_layer_count, const VkSampleCountFlagBits sample_count_flags,
const VkImageUsageFlags image_usage_flags, const VkImageViewType image_view_type,
const VkComponentMapping view_components, const VkImageAspectFlags image_aspect_flags, std::string name
    */

    m_image = std::make_unique<Image>(device, VK_IMAGE_TYPE_2D, format, width, height, 1, 1, VK_SAMPLE_COUNT_1_BIT,
                                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                      VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, name);

    std::vector<VkImageView> attachments = {m_image->image_view()};

    m_framebuffer = std::make_unique<Framebuffer>(device, renderpass, attachments, width, height, name);
}

} // namespace inexor::vulkan_renderer::wrapper
