#pragma once

#include <vk_mem_alloc.h>

#include <string>
#include <utility>
#include <variant>
#include <vector>

// Forward declarations
namespace inexor::vulkan_renderer::wrapper {
class Device;
namespace commands {
class CommandBuffer;
}
} // namespace inexor::vulkan_renderer::wrapper

// Using declarations
using inexor::vulkan_renderer::wrapper::Device;
using inexor::vulkan_renderer::wrapper::commands::CommandBuffer;

namespace inexor::vulkan_renderer::render_graph {

/// RAII base class for resources (buffers and textures)
class ResourceBase {
protected:
    const Device &m_device;
    std::string m_name;

    // Staging buffer is required for updating buffers whose allocation ended up in memory that is not host visible, and
    // staging buffers are also required for every update of textures, because textures are always updated through a
    // staging buffer with a copy command.
    VkBuffer m_staging_buffer{VK_NULL_HANDLE};
    VmaAllocation m_staging_buffer_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_staging_buffer_alloc_info{};

    /// The data required for the update of the buffer or texture resource
    void *m_src_data{nullptr};
    std::size_t m_src_data_size{0};

    virtual void create(const CommandBuffer &cmd_buf) = 0;

    virtual void destroy() = 0;

    /// Call vmaDestroyBuffer
    void destroy_staging_buffer();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The name of the resource
    ResourceBase(const Device &device, std::string name);

    /// Destroy the staging buffer
    virtual ~ResourceBase();

    /// Request an update for this resource
    /// @param src_data A pointer to the source data to update the resource with
    /// @param src_data_size The size of the data
    void request_update(void *src_data, const std::size_t src_data_size);

    /// Request a buffer update
    /// @tparam BufferDataType
    /// @param data
    template <typename BufferDataType>
    void request_update(BufferDataType &data) {
        return request_update(std::addressof(data), sizeof(data));
    }

    /// Request a buffer update
    /// @tparam BufferDataType
    /// @param data
    template <typename BufferDataType>
    void request_update(std::vector<BufferDataType> &data) {
        return request_update(data.data(), sizeof(BufferDataType) * data.size());
    }
};

} // namespace inexor::vulkan_renderer::render_graph
