#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_pool.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorPool::DescriptorPool(const Device &device,
                               std::vector<VkDescriptorPoolSize> pool_sizes,
                               const std::uint32_t max_sets,
                               std::string name)
    : m_device(device), m_pool_sizes(pool_sizes), m_name(std::move(name)) {
    if (m_name.empty()) {
        throw std::invalid_argument("Error: Internal debug name for descriptor pool must not be empty!");
    }
    if (m_pool_sizes.empty()) {
        throw std::invalid_argument("Error: Descriptor pool sizes must not be empty!");
    }

    const auto descriptor_pool_ci = make_info<VkDescriptorPoolCreateInfo>({
        .maxSets = max_sets,
        .poolSizeCount = static_cast<std::uint32_t>(m_pool_sizes.size()),
        .pPoolSizes = m_pool_sizes.data(),
    });

    if (const auto result = vkCreateDescriptorPool(m_device.device(), &descriptor_pool_ci, nullptr, &m_descriptor_pool);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDescriptorPool failed for descriptor pool " + m_name + " !", result);
    }
    m_device.set_debug_name(m_descriptor_pool, m_name);
}

DescriptorPool::DescriptorPool(DescriptorPool &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_descriptor_pool = std::exchange(other.m_descriptor_pool, VK_NULL_HANDLE);
    m_pool_sizes = std::move(other.m_pool_sizes);
}

DescriptorPool::~DescriptorPool() {
    vkDestroyDescriptorPool(m_device.device(), m_descriptor_pool, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
