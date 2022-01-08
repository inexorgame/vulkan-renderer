#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class DescriptorPool {
private:
    const Device &m_device;
    std::string m_name;

    VkDescriptorPool m_descriptor_pool;
    std::vector<VkDescriptorPoolSize> m_pool_sizes;

public:
    DescriptorPool(const Device &device, const std::vector<VkDescriptorPoolSize> &pool_sizes, std::string name);

    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool(DescriptorPool &&) noexcept;
    ~DescriptorPool();

    DescriptorPool &operator=(const DescriptorPool &) = delete;
    DescriptorPool &operator=(DescriptorPool &&) noexcept = default;

    [[nodiscard]] VkDescriptorPool descriptor_pool() const {
        return m_descriptor_pool;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
