#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for resource descriptors.
class ResourceDescriptor {
    std::string m_name;
    const Device &m_device;
    VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};
    VkDescriptorSetLayoutBinding m_descriptor_set_layout_binding;
    VkWriteDescriptorSet m_write_descriptor_set;
    VkDescriptorSet m_descriptor_set;
    std::uint32_t m_swapchain_image_count{0};

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param swapchain_image_count The number of images in swapchain.
    /// @param layout_binding The descriptor layout bindings.
    /// @param descriptor_write The write descriptor sets.
    /// @param name The internal debug marker name of the resource descriptor.
    ResourceDescriptor(const Device &device, std::uint32_t swapchain_image_count,
                       VkDescriptorSetLayoutBinding layout_binding, VkWriteDescriptorSet descriptor_write,
                       std::string name);

    ResourceDescriptor(const ResourceDescriptor &) = default;
    ~ResourceDescriptor();

    [[nodiscard]] const auto &descriptor_set() const {
        return m_descriptor_set;
    }

    [[nodiscard]] auto descriptor_set_layout() const {
        return m_descriptor_set_layout;
    }

    [[nodiscard]] const auto &descriptor_set_layout_bindings() const {
        return m_descriptor_set_layout_binding;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
