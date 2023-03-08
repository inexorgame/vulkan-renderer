#include "inexor/vulkan-renderer/wrapper/renderpass.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

RenderPass::RenderPass(const Device &device, const VkRenderPassCreateInfo &render_pass_ci, std::string name)
    : m_device(device), m_name(std::move(name)) {
    m_device.create_render_pass(render_pass_ci, &m_render_pass, m_name);
}

RenderPass::RenderPass(const Device &device, const std::vector<VkAttachmentDescription> &attachments,
                       const std::vector<VkSubpassDescription> &subpasses,
                       const std::vector<VkSubpassDependency> &dependencies, std::string name)
    : RenderPass(device,
                 wrapper::make_info<VkRenderPassCreateInfo>({
                     .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
                     .pAttachments = attachments.data(),
                     .subpassCount = static_cast<std::uint32_t>(subpasses.size()),
                     .pSubpasses = subpasses.data(),
                     .dependencyCount = static_cast<std::uint32_t>(dependencies.size()),
                     .pDependencies = dependencies.data(),
                 }),
                 std::move(name)) {}

RenderPass::RenderPass(RenderPass &&other) noexcept : m_device(other.m_device) {
    m_render_pass = std::exchange(other.m_render_pass, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(m_device.device(), m_render_pass, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
