#pragma once

// TODO(): Create a vulkan forward declaration header
#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer::wrapper {

class Swapchain;

class Framebuffer {
private:
    VkDevice device;
    VkFramebuffer framebuffer;

public:
    /// Delete the copy constructor so framebuffers are move-only objects.
    Framebuffer(const Framebuffer &) = delete;
    Framebuffer(Framebuffer &&other) noexcept;

    /// Delete the copy assignment operator so framebuffers are move-only objects.
    Framebuffer &operator=(const Framebuffer &) = delete;
    Framebuffer &operator=(Framebuffer &&) noexcept = default;

    /// @brief Creates the frames for the framebuffer.
    /// @param device [in] The Vulkan device.
    /// @param render_pass [in] The render_pass.
    /// @param attachments [in] The framebuffer attachments (image views).
    /// @param swapchain_attachments [in] The swapchain image views.
    /// @param width [in] The width of the framebuffer, mostly equal to the window's width.
    /// @param height [in] The width of the framebuffer, mostly equal to the height's width.
    /// @param swapchain_image_count [in] The number of images in the swapchain.
    /// @param name [in] The internal name of the framebuffer.
    Framebuffer(VkDevice device, VkImageView color, VkImageView depth, VkRenderPass render_pass,
                const wrapper::Swapchain &swapchain);
    ~Framebuffer();

    VkFramebuffer get() const { return framebuffer; }
};

} // namespace inexor::vulkan_renderer::wrapper
