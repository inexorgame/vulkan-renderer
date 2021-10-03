#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <memory>

namespace inexor::vulkan_renderer::wrapper {

class OffscreenFramebuffer {

private:
    std::unique_ptr<Framebuffer> m_framebuffer;

public:
    std::unique_ptr<Image> m_image;

    /// @brief Create the framebuffer and its image.
    /// @param device The device wrapper
    /// @param format The image format
    /// @param width The width of the offscreen framebuffer
    /// @param height The height of the offscreen framebuffer
    /// @param renderpass The VkRenderpass
    /// @param name The name of the offscreen framebuffer
    OffscreenFramebuffer(const wrapper::Device &device, VkFormat format, std::uint32_t width, std::uint32_t height,
                         VkRenderPass renderpass, std::string name);

    [[nodiscard]] auto framebuffer() const {
        return m_framebuffer->framebuffer();
    }

    [[nodiscard]] auto image() const {
        return m_image->image();
    }
};

} // namespace inexor::vulkan_renderer::wrapper
