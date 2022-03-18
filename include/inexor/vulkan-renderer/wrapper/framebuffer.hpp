#pragma once

#include "inexor/vulkan-renderer/wrapper/renderpass.hpp"

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// @brief RAII wrapper class for VkFramebuffer.
class Framebuffer {
    const Device &m_device;
    VkFramebuffer m_framebuffer{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance
    /// @param renderpass The renderpass which is associated with the framebuffer
    /// @param attachments The attachments to use
    /// @param width The width of the framebuffer
    /// @param width The height of the framebuffer
    /// @param name The internal debug marker name of the VkFramebuffer
    Framebuffer(const Device &device, VkRenderPass renderpass, const std::vector<VkImageView> &attachments,
                std::uint32_t width, std::uint32_t height, std::string name);

    Framebuffer(const Device &device, const wrapper::RenderPass &renderpass,
                const std::vector<VkImageView> &attachments, std::uint32_t width, std::uint32_t height,
                std::string name);

    Framebuffer(const Framebuffer &) = delete;
    Framebuffer(Framebuffer &&) noexcept;
    ~Framebuffer();

    Framebuffer &operator=(const Framebuffer &) = delete;
    Framebuffer &operator=(Framebuffer &&) noexcept = default;

    [[nodiscard]] VkFramebuffer framebuffer() const {
        return m_framebuffer;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
