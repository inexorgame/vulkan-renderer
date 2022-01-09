#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include <vulkan/vulkan_core.h>

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

class ResourceDescriptor {
    std::string m_name;
    const Device &m_device;

    std::uint32_t m_descriptor_set_count;

    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSetLayout m_descriptor_set_layout;
    VkDescriptorSet m_descriptor_set;

public:
    ResourceDescriptor(const Device &device, VkDescriptorPool descriptor_pool,
                       std::vector<VkDescriptorSetLayoutBinding> layout_bindings,
                       std::vector<VkWriteDescriptorSet> descriptor_writes, std::string name);

    ResourceDescriptor(const ResourceDescriptor &) = default;
    ResourceDescriptor(ResourceDescriptor &&) noexcept;
    ~ResourceDescriptor();

    ResourceDescriptor &operator=(const ResourceDescriptor &) = default;
    ResourceDescriptor &operator=(ResourceDescriptor &&) noexcept = default;

    [[nodiscard]] VkDescriptorPool descriptor_pool() const {
        return m_descriptor_pool;
    }

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor_set_layout;
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor_set;
    }

    [[nodiscard]] std::uint32_t descriptor_set_count() const {
        return m_descriptor_set_count;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
