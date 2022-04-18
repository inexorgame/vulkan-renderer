#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

class RenderPass {
private:
    const Device &m_device;
    std::string m_name;
    VkRenderPass m_renderpass;

public:
    /// @param device The device wrapper
    /// @param renderpass_ci The renderpass create info structure
    /// @param name The internal name of the renderpass
    RenderPass(const Device &device, const VkRenderPassCreateInfo &renderpass_ci, std::string name);

    RenderPass(const RenderPass &) = delete;
    RenderPass(RenderPass &&) noexcept;
    ~RenderPass();

    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass &operator=(RenderPass &&) noexcept = default;

    [[nodiscard]] VkRenderPass renderpass() const {
        return m_renderpass;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
