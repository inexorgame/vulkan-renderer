#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <array>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Framebuffer::Framebuffer(const Device &device, VkRenderPass render_pass, const std::vector<VkImageView> &attachments,
                         const Swapchain &swapchain, std::string name)
    : m_device(device), m_name(std::move(name)) {
    m_device.create_framebuffer(make_info<VkFramebufferCreateInfo>({
                                    .renderPass = render_pass,
                                    .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
                                    .pAttachments = attachments.data(),
                                    .width = swapchain.extent().width,
                                    .height = swapchain.extent().height,
                                    .layers = 1,
                                }),
                                &m_framebuffer, m_name);
}

Framebuffer::Framebuffer(Framebuffer &&other) noexcept : m_device(other.m_device) {
    m_framebuffer = std::exchange(other.m_framebuffer, nullptr);
    m_name = std::move(other.m_name);
}

Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(m_device.device(), m_framebuffer, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
