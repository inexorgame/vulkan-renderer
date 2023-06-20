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
class DescriptorSetLayoutCache;

/// A builder for descriptors
class DescriptorSetLayoutBuilder {
    friend RenderGraph;

private:
    const Device &m_device;
    DescriptorSetLayoutCache &m_descriptor_set_layout_cache;
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;
    std::uint32_t m_binding{0};

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param descriptor_set_layout_cache The descriptor set layout cache
    DescriptorSetLayoutBuilder(const Device &device, DescriptorSetLayoutCache &descriptor_set_layout_cache);

    DescriptorSetLayoutBuilder(const DescriptorSetLayoutBuilder &) = delete;
    DescriptorSetLayoutBuilder(DescriptorSetLayoutBuilder &&) noexcept;
    ~DescriptorSetLayoutBuilder() = default;

    DescriptorSetLayoutBuilder &operator=(const DescriptorSetLayoutBuilder &) = delete;
    DescriptorSetLayoutBuilder &operator=(DescriptorSetLayoutBuilder &&) = delete;

    // TODO: Support other descriptor types besides uniform buffers and combined image samplers!

    /// Add a combined image sampler to the descriptor set
    /// @param shader_stage The shader stage
    /// @return The dereferenced this pointer
    DescriptorSetLayoutBuilder &add_combined_image_sampler(VkShaderStageFlags shader_stage);

    /// Add a uniform buffer to the descriptor set
    /// @param shader_stage The shader stage
    /// @return The dereferenced this pointer
    DescriptorSetLayoutBuilder &add_uniform_buffer(VkShaderStageFlags shader_stage);

    /// Build the descriptor set layout
    /// @return The descriptor set layout that was created
    [[nodiscard]] VkDescriptorSetLayout build();
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
