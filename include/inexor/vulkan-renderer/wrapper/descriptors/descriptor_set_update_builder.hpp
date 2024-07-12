#pragma once

#include <volk.h>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class Buffer;
enum class BufferType;
class Texture;
} // namespace inexor::vulkan_renderer::render_graph

using inexor::vulkan_renderer::render_graph::Buffer;
using inexor::vulkan_renderer::render_graph::BufferType;
using inexor::vulkan_renderer::render_graph::Texture;

namespace inexor::vulkan_renderer::wrapper::descriptors {

/// A wrapper class for batching calls to vkUpdateDescriptorSets
class DescriptorSetUpdateBuilder {
private:
    const Device &m_device;
    std::vector<VkWriteDescriptorSet> m_write_sets;
    std::uint32_t m_binding{0};

public:
    /// Default constructor
    /// @param device The device wrapper
    DescriptorSetUpdateBuilder(const Device &device);

    DescriptorSetUpdateBuilder(const DescriptorSetUpdateBuilder &) = default;
    DescriptorSetUpdateBuilder(DescriptorSetUpdateBuilder &&) noexcept;
    ~DescriptorSetUpdateBuilder() = default;

    DescriptorSetUpdateBuilder &operator=(const DescriptorSetUpdateBuilder &) = default;
    DescriptorSetUpdateBuilder &operator=(DescriptorSetUpdateBuilder &&) noexcept;

    /// Add a write descriptor set for a uniform buffer
    /// @param descriptor_set The destination descriptor set
    /// @param buffer The rendergraph uniform buffer
    /// @return A reference to the dereferenced ``this`` pointer
    DescriptorSetUpdateBuilder &add_uniform_buffer_update(VkDescriptorSet descriptor_set, std::weak_ptr<Buffer> buffer);

    /// Add a write descriptor set for a combined image sampler
    /// @param descriptor_set The destination descriptor set
    /// @param texture The rendergraph texture
    /// @return A reference to the dereferenced ``this`` pointer
    DescriptorSetUpdateBuilder &add_combined_image_sampler_update(VkDescriptorSet descriptor_set,
                                                                  std::weak_ptr<Texture> texture);

    /// Call vkUpdateDescriptorSets
    void update();
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
