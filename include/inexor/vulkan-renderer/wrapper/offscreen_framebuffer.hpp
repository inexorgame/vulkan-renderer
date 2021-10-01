#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <memory>

namespace inexor::vulkan_renderer::wrapper {

class OffscreenFramebuffer : public Framebuffer, public Image {

public:
    /// @brief Create the framebuffer and its image.
    /// @param device The device wrapper
    /// @param format The image format
    /// @param width The width of the offscreen framebuffer
    /// @param height The height of the offscreen framebuffer
    /// @param renderpass The VkRenderpass
    /// @param attachments The image views
    /// @param name The name of the offscreen framebuffer
    OffscreenFramebuffer(const wrapper::Device &device, VkFormat format, std::uint32_t width, std::uint32_t height,
                         VkRenderPass renderpass, const std::vector<VkImageView> &attachments, std::string name);
};

} // namespace inexor::vulkan_renderer::wrapper
