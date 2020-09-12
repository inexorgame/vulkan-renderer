#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @class ResourceDescriptor
/// @brief RAII wrapper class for resource descriptors.
class ResourceDescriptor {
    const std::string m_name;
    const Device &m_device;
    VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};
    std::vector<VkDescriptorSetLayoutBinding> m_descriptor_set_layout_bindings;
    std::vector<VkWriteDescriptorSet> m_write_descriptor_sets;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    std::uint32_t m_swapchain_image_count{};

public:
    /// @brief Default constructor.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param swapchain_image_count [in] The number of images in swapchain.
    /// @param pool_types [in] The descriptor pool types.
    /// @param layout_bindings [in] The descriptor layout bindings.
    /// @param descriptor_writes [in] The write descriptor sets.
    /// @param name [in] The internal debug marker name of the resource descriptor.
    ResourceDescriptor(const Device &device, std::uint32_t swapchain_image_count,
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
