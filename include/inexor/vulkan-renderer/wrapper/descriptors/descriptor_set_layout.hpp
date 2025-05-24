#pragma once

#include <volk.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Forward declarations
class DescriptorSetLayoutCache;
class DescriptorSetAllocator;

/// RAII wrapper for VkDescriptorPool
/// For internal use inside of rendergraph only!
class DescriptorSetLayout {
    friend DescriptorSetLayoutCache;
    friend DescriptorSetAllocator;
    friend render_graph::RenderGraph;

private:
    const Device &m_device;
    std::string m_name;
    VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};

    /// Default constructor
    /// @param device The device wrapper
    /// @param descriptor_set_layout_ci The descriptor set layout create info
    /// @param name The internal debug name of the descriptor set layout
    DescriptorSetLayout(const Device &device,
                        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci,
                        std::string name);

public:
    /// Call vkDestroyDescriptorSetLayout
    ~DescriptorSetLayout();
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
