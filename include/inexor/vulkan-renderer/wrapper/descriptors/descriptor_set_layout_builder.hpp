#pragma once

#include <volk.h>

#include <string>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

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
    /// @param count The number of combined image samplers
    /// @return The dereferenced this pointer
    DescriptorSetLayoutBuilder &add_combined_image_sampler(VkShaderStageFlags shader_stage, std::uint32_t count = 1);

    /// Add a uniform buffer to the descriptor set
    /// @param shader_stage The shader stage
    /// @param count The number of uniform buffers
    /// @return The dereferenced this pointer
    DescriptorSetLayoutBuilder &add_uniform_buffer(VkShaderStageFlags shader_stage, std::uint32_t count = 1);

    /// Build the descriptor set layout
    /// @param name The name of the descriptor set layout
    /// @return The descriptor set layout that was created
    [[nodiscard]] VkDescriptorSetLayout build(std::string name);
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
