#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Framebuffer::Framebuffer(Framebuffer &&other) noexcept
    : device(other.device), name(std::move(other.name)), frames(std::move(other.frames)) {}

Framebuffer::Framebuffer(const VkDevice device, const VkRenderPass renderpass,
                         std::vector<VkImageView> attachments,
                         const std::vector<VkImageView> &swapchain_attachments, const std::uint32_t width,
                         const std::uint32_t height, const std::uint32_t swapchain_image_count, const std::string name)
    : device(device), name(std::move(name)) {
    assert(device);
    assert(renderpass);
    assert(!name.empty());
    assert(!attachments.empty());
    assert(width > 0);
    assert(height > 0);
    assert(swapchain_image_count > 0);

    spdlog::debug("Creating frame buffers.");
    spdlog::debug("Number of images in swapchain: {}.", swapchain_image_count);

    VkFramebufferCreateInfo framebuffer_ci = {};
    framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_ci.renderPass = renderpass;
    framebuffer_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    framebuffer_ci.pAttachments = attachments.data();
    framebuffer_ci.width = width;
    framebuffer_ci.height = height;
    framebuffer_ci.layers = 1;

    frames.resize(swapchain_image_count);

    // Create one frame buffer for every image in swap chain.
    for (std::size_t i = 0; i < swapchain_image_count; i++) {
        spdlog::debug("Creating framebuffer #{}.", i);

        attachments[0] = swapchain_attachments[i];

        if (vkCreateFramebuffer(device, &framebuffer_ci, nullptr, &frames[i])) {
            throw std::runtime_error("Error: vkCreateFramebuffer failed for framebuffer " + name + " !");
        }
    }

    spdlog::debug("Created frame buffer successfully.");
}

Framebuffer::~Framebuffer() {
    spdlog::trace("Destroying frame buffer {}.", name);

    for (auto *frame : frames) {
        vkDestroyFramebuffer(device, frame, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
