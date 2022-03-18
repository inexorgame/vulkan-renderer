#include "inexor/vulkan-renderer/wrapper/renderpass.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

RenderPass::RenderPass(const Device &device, const VkRenderPassCreateInfo &renderpass_ci, std::string name)
    : m_device(device), m_name(std::move(name)) {
    assert(!m_name.empty());

    if (const auto result = vkCreateRenderPass(device.device(), &renderpass_ci, nullptr, &m_renderpass);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create renderpass for cubemap generation (vkCreateRenderPass)!", result);
    }

    // TODO: Assign internal debug name!
}

RenderPass::RenderPass(RenderPass &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_renderpass = std::exchange(other.m_renderpass, VK_NULL_HANDLE);
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(m_device.device(), m_renderpass, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper