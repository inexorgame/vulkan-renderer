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

using wrapper::Device;

// TODO: Store const reference to rendergraph and retrieve the swapchain image index for automatic buffer tripling

/// RAII wrapper for buffer resources inside of the rendergraph
/// A buffer resource can be a vertex, index, staging, or uniform buffer
class Buffer {
private:
    friend class RenderGraph;

    /// The device wrapper
    const Device &m_device;
    /// The internal name of this buffer resource inside of the rendergraph
    std::string m_name;
    /// The buffer type
    BufferType m_buffer_type;
    /// An optional update function to update the data of the buffer resource
    std::optional<std::function<void()>> m_on_update{std::nullopt};

    VkBuffer m_buffer{VK_NULL_HANDLE};
    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info;
    VkBufferUsageFlags m_buffer_usage;
    VmaMemoryUsage m_mem_usage;

    // TODO: Maybe buffer updates should be done immediately, and no m_src_data should be stored!

    /// It's the responsibility of the programmer to make sure the data m_src_data points to is still valid when
    /// update_buffer() is called!
    void *m_src_data{nullptr};
    std::size_t m_src_data_size{0};

    void create_buffer();
    void update_buffer();
    void destroy_buffer();

public:
    // TODO: Put default constructor into private section?

    /// Default constructor
    /// @param device The device wrapper
    /// @param name The internal debug name of the buffer (must not be empty)
    /// @param usage The internal usage of the buffer in the rendergraph
    /// @note The update frequency of a buffer will only be respected when grouping uniform buffers into descriptor sets
    /// @param on_update An optional update function (``std::nullopt`` by default, meaning no updates to this buffer)
    Buffer(const Device &device, std::string name, BufferType type,
           std::optional<std::function<void()>> on_update = std::nullopt);

    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&other) noexcept;
    ~Buffer();

    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    [[nodiscard]] auto &buffer() const {
        return m_buffer;
    }

    [[nodiscard]] auto &name() const {
        return m_name;
    }

    /// Update the buffer
    /// @param src_data A pointer to the data to copy the updated data from
    /// @warning It is the responsibility of the programmer to make sure src_data still points to valid memory when
    /// update_buffer() is called!
    /// @param src_data_size The size of the data to copy
    void request_update(void *src_data, const std::size_t src_data_size) {
        if (src_data == nullptr) {
            throw std::invalid_argument("Error: Update of buffer resource failed (data pointer is nullptr)!");
        }
        if (src_data_size == 0) {
            throw std::invalid_argument("Error: Update of buffer resource failed (data size is 0)!");
        }
        m_src_data = src_data;
        m_src_data_size = src_data_size;
    }

    template <typename BufferDataType>
    void request_update(BufferDataType &data) {
        return request_update(std::addressof(data), sizeof(data));
    }

    template <typename BufferDataType>
    void request_update(std::vector<BufferDataType> &data) {
        return request_update(data.data(), sizeof(data) * data.size());
    }

    [[nodiscard]] auto type() const {
        return m_buffer_type;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
