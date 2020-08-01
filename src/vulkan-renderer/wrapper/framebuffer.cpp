#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <spdlog/spdlog.h>

#include <array>

namespace inexor::vulkan_renderer::wrapper {

Framebuffer::Framebuffer(VkDevice device, VkRenderPass render_pass, const std::vector<VkImageView> &attachments,
                         const wrapper::Swapchain &swapchain)
    : m_device(device) {
    spdlog::trace("Creating framebuffer");

    auto framebuffer_ci = make_info<VkFramebufferCreateInfo>();
    framebuffer_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    framebuffer_ci.pAttachments = attachments.data();
    framebuffer_ci.width = swapchain.get_extent().width;
    framebuffer_ci.height = swapchain.get_extent().height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = render_pass;

    if (vkCreateFramebuffer(m_device, &framebuffer_ci, nullptr, &m_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer!");
    }
}

Framebuffer::Framebuffer(Framebuffer &&other) noexcept
    : m_device(other.m_device), m_framebuffer(std::exchange(other.m_framebuffer, nullptr)) {}

Framebuffer::~Framebuffer() {
    if (m_framebuffer != VK_NULL_HANDLE) {
        spdlog::trace("Destroying framebuffer");
    }
    vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
