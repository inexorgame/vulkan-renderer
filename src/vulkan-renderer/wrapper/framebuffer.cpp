#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"

#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <spdlog/spdlog.h>

#include <array>

namespace inexor::vulkan_renderer::wrapper {

Framebuffer::Framebuffer(Framebuffer &&other) noexcept
    : device(other.device), framebuffer(std::exchange(other.framebuffer, nullptr)) {}

Framebuffer::Framebuffer(VkDevice device, VkImageView color, VkImageView depth, VkRenderPass render_pass,
                         const wrapper::Swapchain &swapchain)
    : device(device), framebuffer(nullptr) {
    spdlog::trace("Creating framebuffer");

    std::array<VkImageView, 2> attachments = {color, depth};

    // TODO(): Create specialized create info function
    VkFramebufferCreateInfo framebuffer_ci = {};
    framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_ci.attachmentCount = attachments.size();
    framebuffer_ci.pAttachments = attachments.data();
    framebuffer_ci.width = swapchain.get_extent().width;
    framebuffer_ci.height = swapchain.get_extent().height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = render_pass;

    if (vkCreateFramebuffer(device, &framebuffer_ci, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer!");
    }
}

Framebuffer::~Framebuffer() {
    spdlog::trace("Destroying frame buffer");
    vkDestroyFramebuffer(device, framebuffer, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
