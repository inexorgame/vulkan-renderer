#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>
#include <cassert>

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

    /// @brief Creates a renderpass.
    /// @param device [in] The Vulkan device.
    /// @param attachments [in] The renderpass attachments.
    /// @param dependencies [in] The subpass dependencies.
    /// @param subpass_description [in] The subpass description.
    /// @param name [in] The internal name of this renderpass.
    RenderPass(const VkDevice device, const std::vector<VkAttachmentDescription> &attachments,
               const std::vector<VkSubpassDependency> &dependencies, const VkSubpassDescription subpass_description,
               const std::string &name);

    ~RenderPass();

    [[nodiscard]] VkRenderPass get() const {
        assert(device);
        assert(!name.empty());
        assert(renderpass);

        return renderpass;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
