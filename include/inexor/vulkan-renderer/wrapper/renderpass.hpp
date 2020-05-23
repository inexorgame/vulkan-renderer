#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class RenderPass {
private:
    VkDevice device;
    VkRenderPass renderpass;
    std::string name;

public:
    /// Delete the copy constructor so renderpasses are move-only objects.
    RenderPass(const RenderPass &) = delete;
    RenderPass(RenderPass &&other) noexcept;

    /// Delete the copy assignment operator so renderpasses are move-only objects.
    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass &operator=(RenderPass &&) noexcept = default;

    ///
    ///
    ///
    ///
    RenderPass(const VkDevice device, const std::vector<VkAttachmentDescription> &attachments, const std::vector<VkSubpassDependency> &dependencies,
               const VkSubpassDescription subpass_description, const std::string &name);

    ~RenderPass();

    [[nodiscard]] VkRenderPass get() const {
        return renderpass;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
