#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

namespace inexor::vulkan_renderer::wrapper {

class RenderPass {
private:
    const Device &m_device;
    std::string m_name;

    VkRenderPass m_renderpass;

public:
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
