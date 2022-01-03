#include "inexor/vulkan-renderer/texture/sampler.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <utility>

namespace inexor::vulkan_renderer::texture {

Sampler::Sampler(const wrapper::Device &device, const VkSamplerCreateInfo sampler_ci, const std::string name)
    : m_device(device), m_name(name) {

    if (const auto result = vkCreateSampler(m_device.device(), &sampler_ci, nullptr, &m_sampler);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSampler failed for texture " + m_name + " !", result);
    }

    m_device.set_debug_marker_name(m_sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, m_name);
}

Sampler::Sampler(Sampler &&other) noexcept : m_device(other.m_device) {
    m_sampler = other.m_sampler;
    m_name = std::move(other.m_name);
}

Sampler::~Sampler() {
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
}

} // namespace inexor::vulkan_renderer::texture
