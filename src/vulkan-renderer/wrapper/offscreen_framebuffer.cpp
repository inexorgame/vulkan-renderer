#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"

#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

namespace inexor::vulkan_renderer::wrapper {

OffscreenFramebuffer::OffscreenFramebuffer(const wrapper::Device &device, const VkFormat format,
                                           const std::uint32_t width, const std::uint32_t height,
                                           const VkRenderPass renderpass, std::string name)
    : Image(device, VK_IMAGE_TYPE_2D, format, width, height, 1, 1, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, name),
      Framebuffer(device, renderpass, std::vector<VkImageView>{image_view()}, width, height, name) {

    wrapper::OnceCommandBuffer single_command(device);
    single_command.create_command_buffer();
    single_command.start_recording();

    transition_image_layout(single_command.command_buffer(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    single_command.end_recording_and_submit_command();
}

} // namespace inexor::vulkan_renderer::wrapper
