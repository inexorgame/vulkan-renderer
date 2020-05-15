#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"

#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Framebuffer::Framebuffer(Framebuffer &&other) noexcept
    : device(other.device), name(std::move(other.name)), frames(std::move(other.frames)) {}

Framebuffer::Framebuffer(const VkDevice device, const VkRenderPass renderpass, const Swapchain &swapchain,
                         std::vector<VkImageView> attachments)
    : device(device), name("framebuffer") {
    assert(device);
    assert(renderpass);
    assert(!name.empty());

    spdlog::debug("Creating frame buffers.");
    spdlog::debug("Number of images in swapchain: {}.", swapchain.get_image_count());

    VkFramebufferCreateInfo framebuffer_ci = {};
    framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_ci.renderPass = renderpass;
    framebuffer_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    framebuffer_ci.pAttachments = attachments.data();
    framebuffer_ci.width = swapchain.get_extent().width;
    framebuffer_ci.height = swapchain.get_extent().height;
    framebuffer_ci.layers = 1;

    frames.resize(swapchain.get_image_count());

    // Create one frame buffer for every image in swap chain.
    for (std::size_t i = 0; i < swapchain.get_image_count(); i++) {
        spdlog::debug("Creating framebuffer #{}.", i);
        attachments[0] = swapchain.get_image_view(i);
        if (vkCreateFramebuffer(device, &framebuffer_ci, nullptr, &frames[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
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
