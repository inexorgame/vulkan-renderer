#pragma once

#include <volk.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// @brief RAII wrapper class for resource descriptors.
class ResourceDescriptor {
    std::string m_name;
    const Device &m_device;
    VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};
    std::vector<VkDescriptorSetLayoutBinding> m_descriptor_set_layout_bindings;
    std::vector<VkWriteDescriptorSet> m_write_descriptor_sets;
    std::vector<VkDescriptorSet> m_descriptor_sets;

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param layout_bindings The descriptor layout bindings.
    /// @param descriptor_writes The write descriptor sets.
    /// @param name The internal debug marker name of the resource descriptor.
    ResourceDescriptor(const Device &device, std::vector<VkDescriptorSetLayoutBinding> &&layout_bindings,
                       std::vector<VkWriteDescriptorSet> &&descriptor_writes, std::string &&name);

    ResourceDescriptor(const ResourceDescriptor &) = delete;
    ResourceDescriptor(ResourceDescriptor &&) noexcept;
    ~ResourceDescriptor();

    ResourceDescriptor &operator=(const ResourceDescriptor &) = delete;
    ResourceDescriptor &operator=(ResourceDescriptor &&) = delete;

    [[nodiscard]] const auto &descriptor_sets() const {
        return m_descriptor_sets;
    }

    [[nodiscard]] auto descriptor_set_layout() const {
        return m_descriptor_set_layout;
    }

    [[nodiscard]] const auto &descriptor_set_layout_bindings() const {
        return m_descriptor_set_layout_bindings;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
