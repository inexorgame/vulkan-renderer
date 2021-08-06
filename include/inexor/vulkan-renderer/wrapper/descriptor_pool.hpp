#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class DescriptorPool {
private:
    const Device &m_device;
    const std::string m_name;
    std::vector<VkDescriptorPoolSize> m_pool_sizes;
    VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};

public:
    /// @brief
    /// @param
    /// @param
    /// @param
    DescriptorPool(const Device &device, const std::vector<VkDescriptorPoolSize> &pool_sizes, std::string name);
    ~DescriptorPool();

    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool(DescriptorPool &&) = delete;

    [[nodiscard]] const VkDescriptorPool &descriptor_pool() const {
        return m_descriptor_pool;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
