#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @class RenderPass
/// @brief RAII wrapper class for VkRenderPass.
class RenderPass {
    const Device &m_device;
    VkRenderPass m_renderpass;
    std::string m_name;

public:
    /// @brief Default constructor.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param attachments [in] The attachment descriptions.
    /// @param dependencies [in] The subpass dependencies.
    /// @param subpass_description [in] The subpass description.
    /// @param name [in] The internal debug marker name of the VkRenderPass.
    RenderPass(const Device &device, const std::vector<VkAttachmentDescription> &attachments,
               const std::vector<VkSubpassDependency> &dependencies, const VkSubpassDescription subpass_description,
               const std::string &name);

    RenderPass(const RenderPass &) = delete;
    RenderPass(RenderPass &&other) noexcept;

    ~RenderPass();

    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass &operator=(RenderPass &&) noexcept = default;

    [[nodiscard]] VkRenderPass get() const {
        return m_renderpass;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
