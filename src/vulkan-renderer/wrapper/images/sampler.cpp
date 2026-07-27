#include "inexor/vulkan-renderer/wrapper/images/sampler.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::images {

Sampler::Sampler(const Device &device, std::string name, const VkSamplerCreateInfo &sampler_ci)
    : m_device(device), m_name(std::move(name)) {
    if (const auto result = vkCreateSampler(m_device.device(), &sampler_ci, nullptr, &m_sampler);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSampler failed!", result, m_name);
    }
    m_device.set_debug_name(m_sampler, m_name);
}

Sampler::~Sampler() {
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
}

Sampler::Sampler(Sampler &&other) noexcept : m_device(other.m_device) {
    m_sampler = std::exchange(other.m_sampler, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

} // namespace inexor::vulkan_renderer::wrapper::images
