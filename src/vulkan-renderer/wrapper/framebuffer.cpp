#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Framebuffer::Framebuffer(const Device &device, VkRenderPass render_pass, const std::vector<VkImageView> &attachments,
                         const wrapper::Swapchain &swapchain, std::string name)
    : m_device(device), m_name(std::move(name)) {

    auto framebuffer_ci = make_info<VkFramebufferCreateInfo>();
    framebuffer_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    framebuffer_ci.pAttachments = attachments.data();
    framebuffer_ci.width = swapchain.extent().width;
    framebuffer_ci.height = swapchain.extent().height;
    framebuffer_ci.layers = 1;
    framebuffer_ci.renderPass = render_pass;

    if (const auto result = vkCreateFramebuffer(m_device.device(), &framebuffer_ci, nullptr, &m_framebuffer);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create framebuffer " + m_name + "!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, m_name);
}

Framebuffer::Framebuffer(Framebuffer &&other) noexcept : m_device(other.m_device) {
    m_framebuffer = std::exchange(other.m_framebuffer, nullptr);
    m_name = std::move(other.m_name);
}

Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(m_device.device(), m_framebuffer, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
