#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

class RenderPass {
    const Device &m_device;
    VkRenderPass renderpass;
    std::string name;

public:
    /// @brief Creates a renderpass.
    /// @param device [in] A const reference to the Vulkan device wrapper.
    /// @param attachments [in] The renderpass attachments.
    /// @param dependencies [in] The subpass dependencies.
    /// @param subpass_description [in] The subpass description.
    /// @param name [in] The internal name of this renderpass.
    RenderPass(const Device &device, const std::vector<VkAttachmentDescription> &attachments,
               const std::vector<VkSubpassDependency> &dependencies, const VkSubpassDescription subpass_description,
               const std::string &name);

    ~RenderPass();

    RenderPass(const RenderPass &) = delete;
    RenderPass(RenderPass &&other) noexcept;

    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass &operator=(RenderPass &&) noexcept = default;

    [[nodiscard]] VkRenderPass get() const {
        return renderpass;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
