#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for resource descriptors.
class ResourceDescriptor {
    std::string m_name;
    const Device &m_device;
    VkDescriptorSetLayout m_descriptor_set_layout;
    VkDescriptorSetLayoutBinding m_descriptor_set_layout_binding;
    VkWriteDescriptorSet m_write_descriptor_set;
    VkDescriptorSet m_descriptor_set;

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance
    /// @param descriptor_pool The descriptor pool
    /// @param layout_binding The descriptor layout bindings
    /// @param descriptor_write The write descriptor sets
    /// @param name The internal debug marker name of the resource descriptor
    ResourceDescriptor(const Device &device, VkDescriptorPool descriptor_pool,
                       VkDescriptorSetLayoutBinding layout_binding, VkWriteDescriptorSet descriptor_write,
                       std::string name);
    ResourceDescriptor(const ResourceDescriptor &) = delete;
    ResourceDescriptor(ResourceDescriptor &&) = delete;
    ~ResourceDescriptor();

    ResourceDescriptor &operator=(const ResourceDescriptor &) = delete;
    ResourceDescriptor &operator=(ResourceDescriptor &&) = delete;

    [[nodiscard]] const VkDescriptorSet &descriptor_set() const {
        return m_descriptor_set;
    }

    [[nodiscard]] const VkDescriptorSetLayout &descriptor_set_layout() const {
        return m_descriptor_set_layout;
    }

    [[nodiscard]] const VkDescriptorSetLayoutBinding &descriptor_set_layout_bindings() const {
        return m_descriptor_set_layout_binding;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
