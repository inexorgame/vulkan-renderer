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

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Using declarations
using render_graph::Buffer;
using render_graph::BufferType;
using render_graph::Texture;

/// A wrapper class for batching calls to vkUpdateDescriptorSets
class WriteDescriptorSetBuilder {
private:
    const Device &m_device;
    std::vector<VkWriteDescriptorSet> m_write_descriptor_sets;
    std::uint32_t m_binding{0};

    /// Reset the data of the builder so it can be re-used
    void reset();

public:
    /// Default constructor
    /// @param device The device wrapper
    explicit WriteDescriptorSetBuilder(const Device &device);

    WriteDescriptorSetBuilder(const WriteDescriptorSetBuilder &) = default;
    WriteDescriptorSetBuilder(WriteDescriptorSetBuilder &&) noexcept;
    ~WriteDescriptorSetBuilder() = default;

    WriteDescriptorSetBuilder &operator=(const WriteDescriptorSetBuilder &) = default;
    WriteDescriptorSetBuilder &operator=(WriteDescriptorSetBuilder &&) = delete;

    /// Add a write descriptor set for a uniform buffer
    /// @param descriptor_set The destination descriptor set
    /// @param uniform_buffer The rendergraph uniform buffer
    /// @return A reference to the dereferenced ``this`` pointer
    WriteDescriptorSetBuilder &add_uniform_buffer_update(VkDescriptorSet descriptor_set,
                                                         std::weak_ptr<Buffer> uniform_buffer);

    /// Add a write descriptor set for a combined image sampler
    /// @param descriptor_set The destination descriptor set
    /// @param texture_image The rendergraph texture
    /// @return A reference to the dereferenced ``this`` pointer
    WriteDescriptorSetBuilder &add_combined_image_sampler_update(VkDescriptorSet descriptor_set,
                                                                 std::weak_ptr<Texture> texture_image);
    /// Return the write descriptor sets and reset the builder
    /// @return A std::vector of VkWriteDescriptorSet
    [[nodiscard]] std::vector<VkWriteDescriptorSet> build();
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
