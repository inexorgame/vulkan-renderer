#pragma once

#include <vk_mem_alloc.h>

#include <functional>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::Device;

/// The supported buffer types
/// Based on the buffer type, the rendergraph will use Vulkan Memory Allocator to create the buffers
enum class BufferType {
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    // @TODO: Support more buffer types (storage buffer, indirect buffer...)
};

class Buffer {
private:
    // The device wrapper
    const Device &m_device;
    // The buffer name
    std::string m_name;
    // The buffer type
    const BufferType m_type;
    // The buffer update function
    std::function<void()> m_on_update;

    // The Vulkan handle for the buffer
    VkBuffer m_buffer{VK_NULL_HANDLE};
    // The VMA (Vulkan Memory Allocator) allocation
    VmaAllocation m_alloc{VK_NULL_HANDLE};
    // The VMA (Vulkan Memory Allocator) allocation info
    VmaAllocationInfo m_alloc_info{};

    // TODO: Add methods create(), destroy_buffer(), destroy_staging_buffer(), destroy_all()

    /// The descriptor buffer info (required for uniform buffers)
    VkDescriptorBufferInfo m_descriptor_buffer_info;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The buffer name
    /// @param type The buffer type
    /// @param on_update The buffer update function
    Buffer(const Device &device, std::string name, BufferType type, std::function<void()> on_update);

    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&other) noexcept;

    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    [[nodiscard]] auto buffer() const {
        return m_buffer;
    }

    [[nodiscard]] const auto *buffer_address() const {
        return &m_buffer;
    }

    [[nodiscard]] const auto *descriptor_buffer_info() const {
        return &m_descriptor_buffer_info;
    }

    [[nodiscard]] auto name() const {
        return m_name;
    }

    [[nodiscard]] auto type() const {
        return m_type;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
