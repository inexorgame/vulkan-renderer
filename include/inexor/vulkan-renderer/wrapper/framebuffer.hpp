#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;
class Swapchain;

/// @brief RAII wrapper class for VkFramebuffer.
class Framebuffer {
    const wrapper::Device &m_device;
    VkFramebuffer m_framebuffer{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param render_pass The renderpass which is associated with the framebuffer.
    /// @param attachments The attachments to use.
    /// @param swapchain The associated swapchain.
    /// @param name The internal debug marker name of the VkFramebuffer.
    Framebuffer(const Device &device, VkRenderPass render_pass, const std::vector<VkImageView> &attachments,
                const Swapchain &swapchain, std::string name);

    Framebuffer(const Framebuffer &) = delete;
    Framebuffer(Framebuffer &&) noexcept;

    ~Framebuffer();

    Framebuffer &operator=(const Framebuffer &) = delete;
    Framebuffer &operator=(Framebuffer &&) = delete;

    [[nodiscard]] VkFramebuffer get() const {
        return m_framebuffer;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
