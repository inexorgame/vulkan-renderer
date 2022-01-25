#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

class ResourceDescriptor : public DescriptorPool {
    const Device &m_device;

    // TODO: Should we make those mutable, and set them up in a const method?
    VkDescriptorSetLayout m_descriptor_set_layout;
    VkDescriptorSet m_descriptor_set;
    std::string m_name;

public:
    ResourceDescriptor(const Device &device, const std::vector<VkDescriptorPoolSize> &pool_sizes,
                       const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings,
                       std::vector<VkWriteDescriptorSet> &desc_writes, std::string name);

    ResourceDescriptor(const ResourceDescriptor &) = default;
    ResourceDescriptor(ResourceDescriptor &&) noexcept;
    ~ResourceDescriptor();

    ResourceDescriptor &operator=(const ResourceDescriptor &) = default;
    ResourceDescriptor &operator=(ResourceDescriptor &&) noexcept = default;

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor_set_layout;
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor_set;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
