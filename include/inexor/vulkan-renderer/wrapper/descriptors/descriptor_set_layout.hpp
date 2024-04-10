#pragma once

#include <volk.h>

#include <string>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Forward declaration
class DescriptorSetLayoutCache;

/// RAII wrapper for VkDescriptorPool
/// For internal use inside of rendergraph only!
class DescriptorSetLayout {
    friend DescriptorSetLayoutCache;
    // TODO: Make RenderGraph friend

private:
    const Device &m_device;
    std::string m_name;
    VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};

public:
    // TODO: Move me into private again and make rendergraph a friend!

    /// Default constructor
    /// @param device The device wrapper
    /// @param descriptor_set_layout_ci The descriptor set layout create info
    /// @param name The internal debug name of the descriptor set layout
    DescriptorSetLayout(const Device &device, VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci,
                        std::string name);

    DescriptorSetLayout(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout(DescriptorSetLayout &&) noexcept;

    /// Call vkDestroyDescriptorSetLayout
    ~DescriptorSetLayout();

    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout &operator=(DescriptorSetLayout &&) = delete;
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
