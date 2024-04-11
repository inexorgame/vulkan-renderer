#pragma once

#include <vk_mem_alloc.h>

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
/// Forward declarations
class Device;
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

/// The buffer type describes the internal usage of the buffer resource inside of the rendergraph
enum class BufferType {
    STAGING_BUFFER,
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    // TODO: Add more buffer types here and implement support for them
};

/// RAII wrapper for buffer resources inside of the rendergraph
/// A buffer resource can be a vertex, index, or uniform buffer
class Buffer {
private:
    friend class RenderGraph;

    /// The device wrapper
    const wrapper::Device &m_device;
    /// The internal name of this buffer resource inside of the rendergraph
    std::string m_name;
    /// The buffer type
    BufferType m_type;
    /// An optional update function to update the data of the buffer resource
    std::optional<std::function<void()>> m_on_update{std::nullopt};
    /// If this is true, an update can only be carried out with the use of staging buffers
    bool m_requires_staging_buffer_update{false};

    VkBuffer m_buffer{VK_NULL_HANDLE};
    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info;
    VkBufferUsageFlags m_buf_usage;
    VmaMemoryUsage m_mem_usage;

    /// Create the buffer (for internal use in Rendergraph only!)
    /// @param buffer_size The size of the buffer
    /// @param buffer_usage The Vulkan buffer usage flags
    /// @param memory_usage The VMA memory usage
    void create_buffer(VkDeviceSize buffer_size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage);

    /// Destroy the buffer
    void destroy_buffer();

    /// Destroy the buffer and recreate it with the new size
    /// @param new_buffer_size The new size of the buffer
    void recreate_buffer(VkDeviceSize new_buffer_size);

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The internal debug name of the buffer (must not be empty)
    /// @param usage The internal usage of the buffer in the rendergraph
    /// @note The update frequency of a buffer will only be respected when grouping uniform buffers into descriptor sets
    /// @param on_update An optional update function (``std::nullopt`` by default, meaning no updates to this buffer)
    Buffer(const wrapper::Device &device, std::string name, BufferType type,
           std::optional<std::function<void()>> on_update);
    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&other) noexcept;
    ~Buffer();

    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    /// Update the buffer
    /// @param src_data A pointer to the data to copy the updated data from
    /// @param src_data_size The size of the data to copy
    void request_update(void *src_data, const std::size_t src_data_size) {
        if (src_data == nullptr) {
            throw std::invalid_argument("Error: Update of buffer resource failed (data pointer is nullptr)!");
        }
        if (src_data_size == 0) {
            throw std::invalid_argument("Error: Update of buffer resource failed (data size is 0)!");
        }
        // If the new data is bigger than the existing buffer, destroy the buffer and recreate it with the right size
        if (src_data_size > m_alloc_info.size) {
            recreate_buffer(src_data_size);
        }
        if (m_type == BufferType::UNIFORM_BUFFER) {
            // Uniform buffers can be updated simply with memcpy
            std::memcpy(m_alloc_info.pMappedData, src_data, src_data_size);
        }
        // TODO: How to update buffers which are not uniform buffers?
    }

    // TODO: MAYBE WE HAVE TO MOVE THIS INTO RENDERGRAPH AGAIN SO IT CAN DOUBLE OR TRIPLE BUFFER!

    ///
    /// @tparam BufferDataType
    /// @param data
    template <typename BufferDataType>
    void request_update(BufferDataType &data) {
        return request_update(&data, sizeof(data));
    }

    ///
    /// @tparam BufferDataType
    /// @param data
    template <typename BufferDataType>
    void request_update(std::vector<BufferDataType> &data) {
        return request_update(data.data(), sizeof(data) * data.size());
    }

    [[nodiscard]] auto &buffer() const {
        return m_buffer;
    }

    [[nodiscard]] auto &name() const {
        return m_name;
    }

    [[nodiscard]] auto type() const {
        return m_type;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
