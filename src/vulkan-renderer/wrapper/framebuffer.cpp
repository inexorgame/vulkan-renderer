#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Framebuffer::Framebuffer(Framebuffer &&other) noexcept : device(other.device), name(std::move(other.name)), frames(std::move(other.frames)) {}

Framebuffer::Framebuffer(const VkDevice device, const VkRenderPass renderpass, const std::vector<VkImageView> &attachments,
                         const std::vector<VkImageView> &swapchain_attachments, const std::uint32_t width, const std::uint32_t height,
                         const std::uint32_t swapchain_image_count, const bool multisampling_enabled, const std::string &name)
    : device(device), name(name) {
    assert(device);
    assert(renderpass);
    assert(!name.empty());
    assert(!attachments.empty());
    assert(width > 0);
    assert(height > 0);
    assert(swapchain_image_count > 0);

    spdlog::debug("Creating frame buffers.");
    spdlog::debug("Number of images in swapchain: {}.", swapchain_image_count);

    std::vector<VkImageView> framebuffer_attachments = attachments;

    VkFramebufferCreateInfo frame_buffer_ci = {};
    frame_buffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_ci.renderPass = renderpass;
    frame_buffer_ci.attachmentCount = static_cast<std::uint32_t>(framebuffer_attachments.size());
    frame_buffer_ci.pAttachments = framebuffer_attachments.data();
    frame_buffer_ci.width = width;
    frame_buffer_ci.height = height;
    frame_buffer_ci.layers = 1;

    frames.resize(swapchain_image_count);

    // Create one frame buffer for every image in swap chain.
    for (std::size_t i = 0; i < swapchain_image_count; i++) {
        spdlog::debug("Creating framebuffer #{}.", i);

        if (multisampling_enabled) {
            framebuffer_attachments[1] = swapchain_attachments[i];
        } else {
            framebuffer_attachments[0] = swapchain_attachments[i];
        }

        if (vkCreateFramebuffer(device, &frame_buffer_ci, nullptr, &frames[i])) {
            throw std::runtime_error("Error: vkCreateFramebuffer failed for framebuffer " + name + " !");
        }
    }

    spdlog::debug("Created frame buffer successfully.");
}

Framebuffer::~Framebuffer() {
    spdlog::trace("Destroying frame buffer.");

    for (auto *frame : frames) {
        vkDestroyFramebuffer(device, frame, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
