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

/// @brief RAII wrapper class for mesh buffers.
/// In Inexor engine, a "mesh buffer" is a vertex buffer with an associated index buffer.
/// This class bundles vertex buffer and index buffer (if existent).
/// It contains all data which are related to memory allocations for these buffers.
/// @todo Add 'update' method for vertices and indices. If the size of the new vertices/indices
/// exceedes the current memory allocation, we must re-allocate GPU memory.
/// @todo Driver developers recommend that you store multiple buffers, like the vertex and index buffer, into a single
/// VkBuffer and use offsets in commands like vkCmdBindVertexBuffers. The advantage is that your data is more cache
/// friendly in that case, because it’s closer together. It is even possible to reuse the same chunk of memory for
/// multiple resources if they are not used during the same render operations, provided that their data is refreshed, of
/// course. This is known as aliasing and some Vulkan functions have explicit flags to specify that you want to do this.
class MeshBuffer {
    const Device &m_device;
    std::string m_name;

    GPUMemoryBuffer m_vertex_buffer;

    std::optional<GPUMemoryBuffer> m_index_buffer;

    std::uint32_t m_number_of_vertices{0};
    std::uint32_t m_number_of_indices{0};

    // Don't forget that index buffers are optional!
    bool m_index_buffer_available = false;

public:
    /// @brief Construct the mesh buffer and copy the vertex buffers's data and index buffer's data.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the vertex buffer and index buffer.
    /// @param vertex_struct_size The size of the vertex structure.
    /// @param vertex_count The number of vertices in the vertex buffer.
    /// @note The size of the required memory will be calculated by vertex_struct_size * vertex_count.
    /// @param vertices A pointer to the vertex data.
    /// @param index_struct_size The size of the index structure, most likely std::uint32_t.
    /// @param index_count The number of indices in the index buffer.
    /// @param indices  A pointer to the index data.
    MeshBuffer(const Device &device, const std::string &name, VkDeviceSize size_of_vertex_structure,
               std::size_t number_of_vertices, void *vertices, VkDeviceSize size_of_index_structure,
               std::size_t number_of_indices, void *indices);

    /// @brief Construct the mesh buffer and specify the size of the vertex buffer and index buffer.
    /// This constructor will does not yet copy any memory from the vertices or indices!
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the vertex buffer and index buffer.
    /// @param vertex_struct_size The size of the vertex structure.
    /// @param vertex_count The number of vertices in the vertex buffer.
    /// @note The size of the required memory will be calculated by vertex_struct_size * vertex_count.
    /// @param index_struct_size The size of the index structure, most likely std::uint32_t.
    /// @param index_count The number of indices in the index buffer.
    MeshBuffer(const Device &device, const std::string &name, VkDeviceSize size_of_vertex_structure,
               std::size_t number_of_vertices, VkDeviceSize size_of_index_structure, std::size_t number_of_indices);

    /// @brief Construct the vertex buffer and copy the vertex data. The index buffer will not be used!
    /// @warning You should avoid using a vertex buffer without using an associated index buffer!
    /// Not using an index buffer will slow down the performance drastically!
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the vertex buffer and index buffer.
    /// @param vertex_struct_size The size of the vertex structure.
    /// @param vertex_count The number of vertices in the vertex buffer.
    /// @param vertices A pointer to the vertex data.
    MeshBuffer(const Device &device, const std::string &name, VkDeviceSize size_of_vertex_structure,
               std::size_t number_of_vertices, void *vertices);

    /// @brief Construct the vertex buffer and specify it's size. The index buffer will not be used!
    /// @warning You should avoid using a vertex buffer without using an associated index buffer!
    /// Not using an index buffer will slow down the performance drastically!
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal debug marker name of the vertex buffer and index buffer.
    /// @param vertex_struct_size The size of the vertex structure.
    /// @param vertex_count The number of vertices in the vertex buffer.
    MeshBuffer(const Device &device, const std::string &name, VkDeviceSize size_of_vertex_structure,
               std::size_t number_of_vertices);

    MeshBuffer(const MeshBuffer &) = delete;
    MeshBuffer(MeshBuffer &&buffer) noexcept;
    ~MeshBuffer() = default;

    MeshBuffer &operator=(const MeshBuffer &) = delete;
    MeshBuffer &operator=(MeshBuffer &&) = delete;

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
        return m_number_of_vertices;
    }

    [[nodiscard]] std::uint32_t get_index_count() const {
        return m_number_of_indices;
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
