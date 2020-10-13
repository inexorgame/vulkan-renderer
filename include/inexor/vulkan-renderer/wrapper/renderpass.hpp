#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for VkRenderPass.
class RenderPass {
    const Device &m_device;
    VkRenderPass renderpass;
    std::string name;

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param attachments The attachment descriptions.
    /// @param dependencies The subpass dependencies.
    /// @param subpass_description The subpass description.
    /// @param name The internal debug marker name of the VkRenderPass.
    RenderPass(const Device &device, const std::vector<VkAttachmentDescription> &attachments,
               const std::vector<VkSubpassDependency> &dependencies, VkSubpassDescription subpass_description,
               const std::string &name);

    RenderPass(const RenderPass &) = delete;
    RenderPass(RenderPass &&other) noexcept;

    ~RenderPass();

    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass &operator=(RenderPass &&) = delete;

    [[nodiscard]] VkRenderPass get() const {
        return renderpass;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
