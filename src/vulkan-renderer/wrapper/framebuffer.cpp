#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <spdlog/spdlog.h>

#include <array>

namespace inexor::vulkan_renderer::wrapper {

Framebuffer::Framebuffer(const wrapper::Device &device, VkRenderPass render_pass,
                         const std::vector<VkImageView> &attachments, const wrapper::Swapchain &swapchain,
                         const std::string &name)
    : m_device(device), m_name(name) {
    spdlog::trace("Creating framebuffer {}.", m_name);

    auto framebuffer_ci = make_info<VkFramebufferCreateInfo>();
    framebuffer_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    framebuffer_ci.pAttachments = attachments.data();
    framebuffer_ci.width = swapchain.extent().width;
    framebuffer_ci.height = swapchain.extent().height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = render_pass;

    if (vkCreateFramebuffer(m_device.device(), &framebuffer_ci, nullptr, &m_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer " + m_name + "!");
    }

#ifndef NDEBUG
    // Assign an internal name using Vulkan debug markers.
    m_device.set_object_name(reinterpret_cast<std::uint64_t>(m_framebuffer),
                             VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, m_name);
#endif

    spdlog::debug("Created framebuffer {} successfully.", m_name);
}

Framebuffer::Framebuffer(Framebuffer &&other) noexcept
    : m_device(other.m_device), m_framebuffer(std::exchange(other.m_framebuffer, nullptr)),
      m_name(std::move(other.m_name)) {}

Framebuffer::~Framebuffer() {
    spdlog::trace("Destroying framebuffer {}.", m_name);
    vkDestroyFramebuffer(m_device.device(), m_framebuffer, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
