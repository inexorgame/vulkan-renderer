#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Swapchain;

class Framebuffer {
private:
    const wrapper::Device &m_device;
    VkFramebuffer m_framebuffer{VK_NULL_HANDLE};
    const std::string m_name;

public:
    Framebuffer(const Device &device, VkRenderPass render_pass, const std::vector<VkImageView> &attachments,
                const Swapchain &swapchain, const std::string &name);
    Framebuffer(const Framebuffer &) = delete;
    Framebuffer(Framebuffer &&) noexcept;
    ~Framebuffer();

    Framebuffer &operator=(const Framebuffer &) = delete;
    Framebuffer &operator=(Framebuffer &&) = default;

    [[nodiscard]] VkFramebuffer get() const {
        assert(m_framebuffer);
        return m_framebuffer;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
