#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

VkImageCreateInfo OffscreenFramebuffer::make_image_create_info(const VkFormat format, const std::uint32_t width,
                                                               const std::uint32_t height) {
    assert(width > 0);
    assert(height > 0);

    auto image_ci = make_info<VkImageCreateInfo>();

    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format;
    image_ci.extent.width = width;
    image_ci.extent.height = height;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return image_ci;
}

VkImageViewCreateInfo OffscreenFramebuffer::make_image_view_create_info(const VkFormat format) {

    auto image_view_ci = make_info<VkImageViewCreateInfo>();

    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = format;
    image_view_ci.flags = 0;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount = 1;

    // Note that the image member will be filled out later!

    return image_view_ci;
}

OffscreenFramebuffer::OffscreenFramebuffer(const wrapper::Device &device, const VkFormat format,
                                           const std::uint32_t width, const std::uint32_t height,
                                           const VkRenderPass renderpass, std::string name)

    : Image(device, make_image_create_info(format, width, height), make_image_view_create_info(format), name),
      Framebuffer(device, renderpass, std::vector{image_view()}, width, height, name) {}

OffscreenFramebuffer::OffscreenFramebuffer(const wrapper::Device &device, VkFormat format, std::uint32_t width,
                                           std::uint32_t height, const wrapper::RenderPass &renderpass,
                                           std::string name)
    : OffscreenFramebuffer(device, format, width, height, renderpass.renderpass(), std::move(name)) {}

} // namespace inexor::vulkan_renderer::wrapper
