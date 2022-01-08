#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

DescriptorPool::DescriptorPool(const Device &device, const std::vector<VkDescriptorPoolSize> &pool_sizes,
                               std::string name)
    : m_device(device), m_pool_sizes(pool_sizes), m_name(name) {

    assert(!pool_sizes.empty());
    assert(!m_name.empty());

    auto descriptor_pool_ci = wrapper::make_info<VkDescriptorPoolCreateInfo>();
    descriptor_pool_ci.poolSizeCount = static_cast<std::uint32_t>(m_pool_sizes.size());
    descriptor_pool_ci.pPoolSizes = m_pool_sizes.data();

    // TODO: Is this correct?
    descriptor_pool_ci.maxSets = static_cast<std::uint32_t>(pool_sizes.size());

    if (const auto result = vkCreateDescriptorPool(device.device(), &descriptor_pool_ci, nullptr, &m_descriptor_pool);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDescriptorPool failed for descriptor pool " + m_name + " !", result);
    }

    m_device.set_debug_marker_name(m_descriptor_pool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, m_name);
}

DescriptorPool::DescriptorPool(DescriptorPool &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_pool_sizes = std::move(other.m_pool_sizes);
    m_descriptor_pool = std::exchange(other.m_descriptor_pool, VK_NULL_HANDLE);
}

DescriptorPool::~DescriptorPool() {
    vkDestroyDescriptorPool(m_device.device(), m_descriptor_pool, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
