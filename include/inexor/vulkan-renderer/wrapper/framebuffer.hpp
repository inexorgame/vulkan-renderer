#pragma once

// TODO(): Create a vulkan forward declaration header
#include <vulkan/vulkan_core.h>

#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Swapchain;

class Framebuffer {
private:
    VkDevice m_device;
    VkFramebuffer m_framebuffer{VK_NULL_HANDLE};

public:
    Framebuffer(VkDevice device, VkRenderPass render_pass, const std::vector<VkImageView> &attachments,
                const wrapper::Swapchain &swapchain);
    Framebuffer(const Framebuffer &) = delete;
    Framebuffer(Framebuffer &&) noexcept;
    ~Framebuffer();

    Framebuffer &operator=(const Framebuffer &) = delete;
    Framebuffer &operator=(Framebuffer &&) = default;

    [[nodiscard]] VkFramebuffer get() const {
        return m_framebuffer;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
