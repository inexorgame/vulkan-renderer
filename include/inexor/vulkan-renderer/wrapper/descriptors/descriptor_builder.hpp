#pragma once

#include <volk.h>

#include <utility>
#include <vector>

// Forward declaration
namespace inexor::vulkan_renderer {
class RenderGraph;
}

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
}

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Forward declarations
class DescriptorSetAllocator;
class DescriptorSetLayoutCache;

/// A builder for descriptors
class DescriptorBuilder {
    friend RenderGraph;

private:
    const Device &m_device;
    DescriptorSetAllocator &m_descriptor_set_allocator;
    DescriptorSetLayoutCache &m_descriptor_set_layout_cache;
    std::vector<VkWriteDescriptorSet> m_writes;
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;
    std::uint32_t m_binding{0};

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param descriptor_set_allocator The descriptor set layout allocator
    /// @param descriptor_set_layout_cache The descriptor set layout cache
    DescriptorBuilder(const Device &device, DescriptorSetAllocator &descriptor_set_allocator,
                      DescriptorSetLayoutCache &descriptor_set_layout_cache);

    DescriptorBuilder(const DescriptorBuilder &) = delete;
    DescriptorBuilder(DescriptorBuilder &&) noexcept;
    ~DescriptorBuilder() = default;

    DescriptorBuilder &operator=(const DescriptorBuilder &) = delete;
    DescriptorBuilder &operator=(DescriptorBuilder &&) = delete;

    // TODO: Support other descriptor types besides uniform buffers and combined image samplers!

    /// Bind a combined image sampler to the descriptor set
    /// @param image_info The descriptor image info
    /// @param shader_stage The shader stage
    /// @return The dereferenced this pointer
    DescriptorBuilder &bind_image(const VkDescriptorImageInfo *image_info, VkShaderStageFlags shader_stage);

    /// Bind a uniform buffer to the descriptor set
    /// @param buffer_info The descriptor buffer info
    /// @param shader_stage The shader stage
    /// @return The dereferenced this pointer
    DescriptorBuilder &bind_uniform_buffer(const VkDescriptorBufferInfo *buffer_info, VkShaderStageFlags shader_stage);

    /// Build the descriptor set layout and allocate the descriptor set
    /// @return A std::pair of the descriptor set and the descriptor set layout
    [[nodiscard]] std::pair<VkDescriptorSet, VkDescriptorSetLayout> build();
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
