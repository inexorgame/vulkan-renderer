#pragma once

#include "inexor/vulkan-renderer/render-graph/resource_base.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::descriptors {
// Forward declaration
class WriteDescriptorSetBuilder;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::render_modules {
// Forward declaration
class RenderModuleBase;
} // namespace inexor::vulkan_renderer::render_modules

namespace inexor::vulkan_renderer::render_graph {

/// The buffer type describes the internal usage of the buffer resource inside of the rendergraph
enum class BufferType {
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    // TODO: Support more buffer types (storage buffer, indirect buffer..)
};

// Forward declaration
class GraphicsPass;
class RenderGraph;

// Using declarations
using render_modules::RenderModuleBase;
using wrapper::Device;
using wrapper::commands::CommandBuffer;
using wrapper::descriptors::WriteDescriptorSetBuilder;

// TODO: Store const reference to rendergraph and retrieve the swapchain image index for automatic buffer tripling

/// RAII wrapper for buffer resources inside of the rendergraph
/// A buffer resource can be a vertex buffer, index buffer, or uniform buffer
class BufferResource : public ResourceBase {
    friend RenderGraph;
    friend WriteDescriptorSetBuilder;

private:
    /// The buffer type will be set depending on which constructor of the Buffer wrapper is called by rendergraph. The
    /// engine currently supports three different types of buffers in the Buffer wrapper class: vertex buffers, index
    /// buffers, and uniform buffers. The instances of the Buffer wrapper class are managed by rendergraph only. One
    /// solution to deal with the different buffer types would be to use a BufferBase class and to make three distinct
    /// classes VertexBuffer, IndexBuffer, and UniformBuffer. However, we aimed for simplicity and wanted to avoid
    /// polymorphism in the rendergraph for performance reasons. We also refrained from using templates for this use
    /// case. Therefore, we have chosen to use only one Buffer wrapper class which contains members for all three
    /// different buffer types. The type of the buffer will be set depending on which Buffer constructor is called by
    /// rendergraph. The actual memory management for the buffers is done by Vulkan Memory Allocator (VMA) internally.
    BufferType m_buffer_type;

    // The buffer resource and VMA wrapper data
    VkBuffer m_buffer{VK_NULL_HANDLE};
    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info{};

    // This is required for uniform buffers only
    VkDescriptorBufferInfo m_descriptor_buffer_info{};

    /// Default constructor
    /// @param device The device wrapper
    /// @param name The name of the buffer
    /// @param type The type of the buffer
    BufferResource(const Device &device, std::string name, BufferType type);

    ///
    ///
    void create(const CommandBuffer &cmd_buf) override;

    /// Call vmaDestroyBuffer
    void destroy() override;

    /// Indicates to the rendergraph if an udpate is requested from external code
    [[nodiscard]] bool is_update_requested() const {
        return m_src_data != nullptr;
    }

public:
    /// Call destroy_buffer
    ~BufferResource();
};

} // namespace inexor::vulkan_renderer::render_graph
