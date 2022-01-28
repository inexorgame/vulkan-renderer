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
private:
    const Device &m_device;
    std::string m_name;

    void create_descriptor_set_layout(const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings) const;
    void allocate_descriptor_set(std::vector<VkWriteDescriptorSet> &desc_writes) const;

public:
    mutable VkDescriptorSetLayout descriptor_set_layout;
    mutable VkDescriptorSet descriptor_set;

    ResourceDescriptor(const Device &device, const std::vector<VkDescriptorPoolSize> &pool_sizes,
                       const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings,
                       std::vector<VkWriteDescriptorSet> &desc_writes, std::string name);

    ResourceDescriptor(const ResourceDescriptor &) = default;
    ResourceDescriptor(ResourceDescriptor &&) noexcept;
    ~ResourceDescriptor();

    ResourceDescriptor &operator=(const ResourceDescriptor &) = default;
    ResourceDescriptor &operator=(ResourceDescriptor &&) noexcept = default;
};

} // namespace inexor::vulkan_renderer::wrapper
