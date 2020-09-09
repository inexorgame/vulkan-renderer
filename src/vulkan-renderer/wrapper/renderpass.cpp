#include "inexor/vulkan-renderer/wrapper/renderpass.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

RenderPass::RenderPass(RenderPass &&other) noexcept
    : m_device(other.m_device), m_renderpass(std::exchange(other.m_renderpass, nullptr)),
      m_name(std::move(other.m_name)) {}

RenderPass::RenderPass(const Device &device, const std::vector<VkAttachmentDescription> &attachments,
                       const std::vector<VkSubpassDependency> &dependencies,
                       const VkSubpassDescription subpass_description, const std::string &name)
    : m_device(device), m_name(m_name) {

    VkRenderPassCreateInfo renderpass_ci = {};
    renderpass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    renderpass_ci.pAttachments = attachments.data();
    renderpass_ci.subpassCount = 1;
    renderpass_ci.pSubpasses = &subpass_description;
    renderpass_ci.dependencyCount = static_cast<std::uint32_t>(dependencies.size());
    renderpass_ci.pDependencies = dependencies.data();

    spdlog::debug("Creating renderpass {}.", name);

    if (vkCreateRenderPass(m_device.device(), &renderpass_ci, nullptr, &m_renderpass) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateRenderPass failed for " + name + " !");
    }

    spdlog::debug("Created renderpass successfully.");
}

RenderPass::~RenderPass() {
    if (m_renderpass != nullptr) {
        spdlog::trace("Destroying render pass {}.", m_name);
        vkDestroyRenderPass(m_device.device(), m_renderpass, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
