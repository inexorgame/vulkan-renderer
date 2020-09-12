#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"

#include <vma/vma_usage.h>
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @class MeshBuffer
/// @brief RAII wrapper class for mesh buffers.
/// In Inexor engine, a mesh buffer is a vertex buffer with an associated index buffer.
/// @todo Add 'update' method for vertices and indices. If the size of the new vertices/indices
/// exceedes the current memory allocation, we must re-allocate GPU memory.
class MeshBuffer {
    const Device &m_device;
    GPUMemoryBuffer m_vertex_buffer;
    std::string m_name;
    std::uint32_t m_vertex_count{0};
    std::uint32_t m_index_count{0};
    std::optional<GPUMemoryBuffer> m_index_buffer;

    // Don't forget that index buffers are optional!
    bool m_index_buffer_available = false;

public:
    /// @brief Constructs the mesh buffer by copying the vertexb buffers's data and index buffer's data.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param name [in] The internal debug marker name of the vertex buffer and index buffer.
    /// @param vertex_struct_size [in] The size of the vertex structure.
    /// @param vertex_count [in] The number of vertices in the vertex buffer.
    /// @note The size of the required memory will be calculated by vertex_struct_size * vertex_count.
    /// @param vertices [in] A pointer to the vertex data.
    /// @param index_struct_size [in] The size of the index structure, most likely std::uint32_t.
    /// @param index_count [in] The number of indices in the index buffer.
    /// @param indices [in]  A pointer to the index data.
    MeshBuffer(const Device &device, const std::string &name, const VkDeviceSize vertex_struct_size,
               const std::size_t vertex_count, void *vertices, const VkDeviceSize index_struct_size,
               const std::size_t index_count, void *indices);

    /// @brief Constructs the mesh buffer by specifying the size of the vertex buffer and index buffer.
    /// This constructor will does not yet copy any memory from the vertices or indices!
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param name [in] The internal debug marker name of the vertex buffer and index buffer.
    /// @param vertex_struct_size [in] The size of the vertex structure.
    /// @param vertex_count [in] The number of vertices in the vertex buffer.
    /// @note The size of the required memory will be calculated by vertex_struct_size * vertex_count.
    /// @param index_struct_size [in] The size of the index structure, most likely std::uint32_t.
    /// @param index_count [in] The number of indices in the index buffer.
    MeshBuffer(const Device &device, const std::string &name, const VkDeviceSize vertex_struct_size,
               const std::size_t vertex_count, const VkDeviceSize index_struct_size, const std::size_t index_count);

    /// @brief Constructs the vertex buffer by copying the vertex data. The index buffer will not be used.
    /// @warning You should avoid using a vertex buffer without using an associated index buffer!
    /// Not using an index buffer will slow down the performance drastically!
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param name [in] The internal debug marker name of the vertex buffer and index buffer.
    /// @param vertex_struct_size [in] The size of the vertex structure.
    /// @param vertex_count [in] The number of vertices in the vertex buffer.
    /// @param vertices [in] A pointer to the vertex data.
    MeshBuffer(const Device &device, const std::string &name, const VkDeviceSize vertex_struct_size,
               const std::size_t vertex_count, void *vertices);

    /// @brief Constructs the vertex buffer by specifying it's size. The index buffer will not be used.
    /// @warning You should avoid using a vertex buffer without using an associated index buffer!
    /// Not using an index buffer will slow down the performance drastically!
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param name [in] The internal debug marker name of the vertex buffer and index buffer.
    /// @param vertex_struct_size [in] The size of the vertex structure.
    /// @param vertex_count [in] The number of vertices in the vertex buffer.
    MeshBuffer(const Device &device, const std::string &name, const VkDeviceSize vertex_struct_size,
               const std::size_t vertex_count);

    MeshBuffer(const MeshBuffer &) = delete;
    MeshBuffer(MeshBuffer &&buffer) noexcept;

    MeshBuffer &operator=(const MeshBuffer &) = delete;
    MeshBuffer &operator=(MeshBuffer &&) = default;

    [[nodiscard]] VkBuffer get_vertex_buffer() const {
        return m_vertex_buffer.buffer();
    }

    [[nodiscard]] bool has_index_buffer() const {
        return m_index_buffer.has_value();
    }

    [[nodiscard]] VkBuffer get_index_buffer() const {
        return m_index_buffer.value().buffer();
    }

    [[nodiscard]] std::uint32_t get_vertex_count() const {
        return m_vertex_count;
    }

    [[nodiscard]] std::uint32_t get_index_count() const {
        return m_index_count;
    }

    [[nodiscard]] auto get_vertex_buffer_address() const {
        return m_vertex_buffer.allocation_info().pMappedData;
    }

    [[nodiscard]] auto get_index_buffer_address() const {
        if (!m_index_buffer) {
            throw std::runtime_error(std::string("Error: No index buffer for mesh " + m_name + "!"));
        }

        return m_index_buffer.value().allocation_info().pMappedData;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
