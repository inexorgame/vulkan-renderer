#pragma once

#include <volk.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// RAII wrapper for VkRenderPass
class RenderPass {
private:
    const Device &m_device;
    VkRenderPass m_render_pass{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param render_pass_ci The render pass create info
    /// @param name The internal debug name of the render pass (must not be empty)
    /// @exception std::invalid_argument The internal debug name of the renderpass is empty
    /// @exception VulkanException vkCreateRenderPass call failed
    RenderPass(const Device &device, const VkRenderPassCreateInfo &render_pass_ci, std::string name);

    /// Overloaded constructor which takes the attachment descriptions, subpass descriptions, and subpass dependencies
    /// @param device The device wrapper
    /// @param render_pass_ci The render pass create info
    /// @param attachments The attachment descriptions
    /// @param subpasses The subpass descriptions
    /// @param dependencies The dependencies
    /// @param name The internal debug name of the render pass (must not be empty)
    /// @exception std::invalid_argument The internal debug name of the renderpass is empty
    /// @exception VulkanException vkCreateRenderPass call failed
    RenderPass(const Device &device, const std::vector<VkAttachmentDescription> &attachments,
               const std::vector<VkSubpassDescription> &subpasses, const std::vector<VkSubpassDependency> &dependencies,
               std::string name);

    RenderPass(const RenderPass &) = delete;
    RenderPass(RenderPass &&) noexcept;
    ~RenderPass();

    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass &operator=(RenderPass &&) = delete;

    [[nodiscard]] VkRenderPass render_pass() const noexcept {
        return m_render_pass;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
