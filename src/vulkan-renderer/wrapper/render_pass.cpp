#include "inexor/vulkan-renderer/wrapper/render_pass.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

RenderPass::RenderPass(const Device &device, const VkRenderPassCreateInfo &render_pass_ci, std::string name)
    : m_device(device), m_name(std::move(name)) {
    if (m_name.empty()) {
        throw std::invalid_argument("Error: renderpass name must not be empty!");
    }

    if (const auto result = vkCreateRenderPass(m_device.device(), &render_pass_ci, nullptr, &m_render_pass);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateRenderPass failed for renderpass " + m_name + " !", result);
    }
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
