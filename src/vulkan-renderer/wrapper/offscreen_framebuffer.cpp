#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"

namespace inexor::vulkan_renderer::wrapper {

OffscreenFramebuffer::OffscreenFramebuffer(const wrapper::Device &device, const VkFormat format,
                                           const std::uint32_t width, const std::uint32_t height,
                                           const VkRenderPass renderpass, const std::vector<VkImageView> &attachments,
                                           std::string name)

    : Framebuffer(device, renderpass, attachments, width, height, name),

      Image(device, VK_IMAGE_TYPE_2D, format, width, height, 1, 1, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_VIEW_TYPE_2D,
            VK_IMAGE_ASPECT_COLOR_BIT, name) {}

} // namespace inexor::vulkan_renderer::wrapper
