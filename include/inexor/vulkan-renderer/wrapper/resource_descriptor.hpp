#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

/// @brief A class for descriptor management.
/// Shader access of data is managed through descriptors.
/// Descriptors are organized in descriptor sets.
/// Descriptors sets are described through their descriptor set layout.
/// Descriptor sets are allocated from descriptor pools.
class ResourceDescriptor {
private:
    std::string m_name;
    VkDevice m_device{VK_NULL_HANDLE};
    VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};
    std::vector<VkDescriptorSetLayoutBinding> m_descriptor_set_layout_bindings;
    std::vector<VkWriteDescriptorSet> m_write_descriptor_sets;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    std::uint32_t m_swapchain_image_count{};

public:
    // @note Creates a descriptor pool.
    ResourceDescriptor(VkDevice device, std::uint32_t swapchain_image_count,
                       std::initializer_list<VkDescriptorType> pool_types,
                       const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings,
                       const std::vector<VkWriteDescriptorSet> &descriptor_writes, const std::string &name);

    ResourceDescriptor(const ResourceDescriptor &) = delete;
    ResourceDescriptor(ResourceDescriptor &&) noexcept;
    ~ResourceDescriptor();

    ResourceDescriptor &operator=(const ResourceDescriptor &) = delete;
    ResourceDescriptor &operator=(ResourceDescriptor &&) noexcept = default;

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
