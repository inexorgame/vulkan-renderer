#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"

#include <volk.h>

#include <memory>
#include <variant>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {
// Forward declarations
class BufferResource;
enum class BufferType;
class RenderGraph;
class TextureResource;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Using declarations
using render_graph::BufferResource;
using render_graph::BufferType;
using render_graph::RenderGraph;
using render_graph::TextureResource;

/// A wrapper class for batching calls to vkUpdateDescriptorSets
class WriteDescriptorSetBuilder {
    friend class RenderGraph;

private:
    const Device &m_device;
    std::vector<VkWriteDescriptorSet> m_write_descriptor_sets;
    std::uint32_t m_binding;

    /// Return the write descriptor sets and reset the builder
    /// @return A std::vector of VkWriteDescriptorSet
    [[nodiscard]] std::vector<VkWriteDescriptorSet> build();

public:
    /// Default constructor
    /// @param device The device wrapper
    WriteDescriptorSetBuilder(const Device &device);

    /// Add a new entry to the write descriptor set builder
    /// @param descriptor_set The descriptor set
    /// @param descriptor_data Either a buffer or a texture
    /// @param descriptor_count The descriptor count (``1`` by default)
    [[nodiscard]] WriteDescriptorSetBuilder &
    add(const VkDescriptorSet descriptor_set,
        std::variant<std::weak_ptr<TextureResource>, std::weak_ptr<BufferResource>> descriptor_data,
        std::uint32_t descriptor_count = 1);
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
