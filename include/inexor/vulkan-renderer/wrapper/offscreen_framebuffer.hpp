#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <memory>

namespace inexor::vulkan_renderer::wrapper {

class OffscreenFramebuffer : public Image, public Framebuffer {
private:
    ///
    ///
    ///
    ///
    [[nodiscard]] VkImageCreateInfo make_image_create_info(VkFormat format, std::uint32_t width, std::uint32_t height);

    ///
    ///
    [[nodiscard]] VkImageViewCreateInfo make_image_view_create_info(VkFormat format);

public:
    OffscreenFramebuffer(const wrapper::Device &device, VkFormat format, std::uint32_t width, std::uint32_t height,
                         VkRenderPass renderpass, std::string name);
};

} // namespace inexor::vulkan_renderer::wrapper
