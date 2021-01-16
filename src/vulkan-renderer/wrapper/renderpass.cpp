#include "inexor/vulkan-renderer/wrapper/renderpass.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

RenderPass::RenderPass(RenderPass &&other) noexcept
    : m_device(other.m_device), renderpass(std::exchange(other.renderpass, nullptr)), name(std::move(other.name)) {}

RenderPass::RenderPass(const Device &device, const std::vector<VkAttachmentDescription> &attachments,
                       const std::vector<VkSubpassDependency> &dependencies,
                       const VkSubpassDescription subpass_description, const std::string &name)
    : m_device(device), name(name) {

    VkRenderPassCreateInfo renderpass_ci = {};
    renderpass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    renderpass_ci.pAttachments = attachments.data();
    renderpass_ci.subpassCount = 1;
    renderpass_ci.pSubpasses = &subpass_description;
    renderpass_ci.dependencyCount = static_cast<std::uint32_t>(dependencies.size());
    renderpass_ci.pDependencies = dependencies.data();

    spdlog::debug("Creating renderpass {}.", name);

    if (const auto result = vkCreateRenderPass(m_device.device(), &renderpass_ci, nullptr, &renderpass);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateRenderPass failed for " + name + " !", result);
    }

    spdlog::debug("Created renderpass successfully.");
}

RenderPass::~RenderPass() {
    spdlog::trace("Destroying render pass {}.", name);
    vkDestroyRenderPass(m_device.device(), renderpass, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
