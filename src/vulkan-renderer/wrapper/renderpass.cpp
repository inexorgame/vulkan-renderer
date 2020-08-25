#include "inexor/vulkan-renderer/wrapper/renderpass.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

RenderPass::RenderPass(RenderPass &&other) noexcept
    : device(other.device), renderpass(std::exchange(other.renderpass, nullptr)), name(std::move(other.name)) {}

RenderPass::RenderPass(const VkDevice device, const std::vector<VkAttachmentDescription> &attachments,
                       const std::vector<VkSubpassDependency> &dependencies,
                       const VkSubpassDescription subpass_description, const std::string &name)
    : device(device), name(name) {

    VkRenderPassCreateInfo renderpass_ci = {};
    renderpass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    renderpass_ci.pAttachments = attachments.data();
    renderpass_ci.subpassCount = 1;
    renderpass_ci.pSubpasses = &subpass_description;
    renderpass_ci.dependencyCount = 2;
    renderpass_ci.pDependencies = dependencies.data();

    spdlog::debug("Creating renderpass {}.", name);

    if (vkCreateRenderPass(device, &renderpass_ci, nullptr, &renderpass) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateRenderPass failed for " + name + " !");
    }

    spdlog::debug("Created renderpass successfully.");
}

RenderPass::~RenderPass() {
    spdlog::trace("Destroying render pass {}.", name);
    vkDestroyRenderPass(device, renderpass, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
