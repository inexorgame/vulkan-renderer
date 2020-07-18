#pragma once

#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Framebuffer {
private:
    VkDevice device;
    std::vector<VkFramebuffer> frames;
    std::string name;

public:
    /// Delete the copy constructor so framebuffers are move-only objects.
    Framebuffer(const Framebuffer &) = delete;
    Framebuffer(Framebuffer &&other) noexcept;

    /// Delete the copy assignment operator so framebuffers are move-only objects.
    Framebuffer &operator=(const Framebuffer &) = delete;
    Framebuffer &operator=(Framebuffer &&) noexcept = default;

    /// @brief Creates the frames for the framebuffer.
    /// @todo Currently, 4xMSAA is used as default multisampling. Adapt this code once that changes.
    /// @param device [in] The Vulkan device.
    /// @param renderpass [in] The renderpass.
    /// @param attachments [in] The framebuffer attachments (image views).
    /// @param swapchain_attachments [in] The swapchain image views.
    /// @param width [in] The width of the framebuffer, mostly equal to the window's width.
    /// @param height [in] The width of the framebuffer, mostly equal to the height's width.
    /// @param swapchain_image_count [in] The number of images in the swapchain.
    /// @param multisampling_enabled [in] True if multisampling is enabled, false otherwise.
    /// @param name [in] The internal name of the framebuffer.
    Framebuffer(const VkDevice device, const VkRenderPass renderpass, std::vector<VkImageView> attachments,
                const std::vector<VkImageView> &swapchain_attachments, const std::uint32_t width,
                const std::uint32_t height, const std::uint32_t swapchain_image_count, const bool multisampling_enabled,
                const std::string name);

    ~Framebuffer();

    [[nodiscard]] VkFramebuffer get(const std::size_t index) const {
        if (index >= frames.size()) {
            throw std::out_of_range("Error: Index " + std::to_string(index) +
                                    " for frame buffers is out of range! Size: " + std::to_string(frames.size()));
        }
        return frames[index];
    }
};

} // namespace inexor::vulkan_renderer::wrapper
