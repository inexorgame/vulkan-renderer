#include "inexor/vulkan-renderer/wrapper/sampler.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Sampler::Sampler(const Device &device, std::string name, const VkSamplerCreateInfo &sampler_ci)
    : m_device(device), m_name(std::move(name)) {
    if (const auto result = vkCreateSampler(m_device.device(), &sampler_ci, nullptr, &m_sampler);
        result != VK_SUCCESS) {
        throw VulkanException("[Sampler::Sampler] Error: vkCreateSampler failed for sampler " + m_name + " !", result);
    }
    m_device.set_debug_name(m_sampler, m_name);
}

Sampler::~Sampler() {
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
