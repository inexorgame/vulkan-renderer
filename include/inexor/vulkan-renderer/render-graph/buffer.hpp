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
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    // TODO: Add more buffer types here and implement support for them
};

using wrapper::Device;

// TODO: Store const reference to rendergraph and retrieve the swapchain image index for automatic buffer tripling

/// RAII wrapper for buffer resources inside of the rendergraph
/// A buffer resource can be a vertex buffer, index buffer, or uniform buffer
class Buffer {
private:
    friend class RenderGraph;

    /// The device wrapper
    const Device &m_device;
    /// The internal debug name of the buffer resource
    std::string m_name;

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

    /// The buffer update function which is called by rendergraph to update the buffer's data. This update function is
    /// called, no matter what the type of the buffer is. With the currently supported buffer types (vertex-, index-,
    /// and uniform buffers) there is always a discussion about whether some update lambdas can be made std::optional.
    /// For example we could have one vertex buffer with an index buffer and the index buffer is updated together with
    /// the vertex buffer in the update function of the vertex buffer. From the design of the engine there is no
    /// limitation which buffer is updated in which update function, as long as the handle to that buffer has been
    /// created in rendergraph. In our exxample, the update function of the index buffer could be std::nullopt. In this
    /// case, rendergraph could separate all buffers into those which require an update and those who do not. For
    /// simplicity however, we made the update function not std::optional.
    std::optional<std::function<void()>> m_on_update;

    /// This is relevant for index buffers only
    VkIndexType m_index_type;
    /// This is relevant for vertex buffers only
    std::vector<VkVertexInputAttributeDescription> m_vert_input_attr_descs;
    /// TODO: Is this is relevant for uniform buffers only?
    /// TODO: Maybe buffer updates should be done immediately, and no m_src_data should be stored!
    /// It's the responsibility of the programmer to make sure the data m_src_data points to is still valid when
    /// update_buffer() is called!
    void *m_src_data{nullptr};
    std::size_t m_src_data_size{0};

    /// The resources for actual memory management of the buffer
    VkBuffer m_buffer{VK_NULL_HANDLE};
    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info{};
    VkBufferUsageFlags m_buffer_usage{};
    VmaMemoryUsage m_mem_usage{};

    /// Create the buffer using Vulkan Memory Allocator (VMA) library
    void create_buffer();

    void update_buffer();

    /// Call vmaDestroyBuffer
    void destroy_buffer();

public:
    /// Constructor for uniform buffers
    /// @param device The device wrapper
    /// @param name The name of the buffer
    /// @param on_update The buffer update function (``std::nullopt`` by default)
    Buffer(const Device &device, std::string name, std::optional<std::function<void()>> on_update = std::nullopt);

    /// Constructor for vertex buffers
    /// @param device The device wrapper
    /// @param vert_input_attr_descs A vertex of vertex input attribute descriptions
    /// @param on_update The buffer update function (``std::nullopt`` by default)
    Buffer(const Device &device, std::string name, std::vector<VkVertexInputAttributeDescription> vert_input_attr_descs,
           std::optional<std::function<void()>> on_update = std::nullopt);

    /// Constructor for index buffers
    /// @param device The device wrapper
    /// @param name The name of the buffer
    /// @param index_type The Vulkan index type
    /// @param on_update The buffer update function (``std::nullopt`` by default)
    Buffer(const Device &device, std::string name, VkIndexType index_type,
           std::optional<std::function<void()>> on_update = std::nullopt);

    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&other) noexcept;
    ~Buffer();

    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    // TODO: Remove get methods and allow access to private fields through friend class declaration only?
    [[nodiscard]] auto &buffer() const {
        return m_buffer;
    }

    // TODO: Remove get methods and allow access to private fields through friend class declaration only?
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
