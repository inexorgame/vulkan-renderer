#pragma once

#include "inexor/vulkan-renderer/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/once_command_buffer.hpp"
#include "inexor/vulkan-renderer/staging_buffer.hpp"

#include <vma/vma_usage.h>
#include <vulkan/vulkan.h>

#include <optional>

namespace inexor::vulkan_renderer {

/// @brief A structure which bundles vertex buffer and index buffer (if existent).
/// It contains all data which are related to memory allocations for these buffers.
/// @todo Driver developers recommend that you store multiple
/// buffers, like the vertex and index buffer, into a single VkBuffer and use offsets
/// in commands like vkCmdBindVertexBuffers. The advantage is that your data
/// is more cache friendly in that case, because it’s closer together. It is even possible
/// to reuse the same chunk of memory for multiple resources if they are not
/// used during the same render operations, provided that their data is refreshed,
/// of course. This is known as aliasing and some Vulkan functions have explicit
/// flags to specify that you want to do this.
class MeshBuffer {

private:
    std::string name = "";

    GPUMemoryBuffer vertex_buffer;

    // Index buffer, if available.
    std::optional<GPUMemoryBuffer> index_buffer;

    std::uint32_t number_of_vertices = 0;

    std::uint32_t number_of_indices = 0;

    // Don't forget that index buffers are optional!
    bool index_buffer_available = false;

    OnceCommandBuffer copy_command_buffer;

public:
    // Delete the copy constructor so mesh buffers are move-only objects.
    MeshBuffer(const MeshBuffer &) = delete;
    MeshBuffer(MeshBuffer &&buffer) noexcept;

    // Delete the copy assignment operator so uniform buffers are move-only objects.
    MeshBuffer &operator=(const MeshBuffer &) = delete;
    MeshBuffer &operator=(MeshBuffer &&) noexcept = default;

    /// @brief Creates a new vertex buffer and an associated index buffer.
    MeshBuffer(const VkDevice device, VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index, const VmaAllocator vma_allocator, std::string name,
               const VkDeviceSize size_of_vertex_structure, const std::uint32_t number_of_vertices, void *vertices, const VkDeviceSize size_of_index_structure,
               const std::uint32_t number_of_indices, void *indices);

    /// @brief Creates a vertex buffer without index buffer.
    MeshBuffer(const VkDevice device, VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index, const VmaAllocator vma_allocator, std::string name,
               const VkDeviceSize size_of_vertex_structure, const std::uint32_t number_of_vertices, void *vertices);

    VkBuffer get_vertex_buffer() const {
        return vertex_buffer.get_buffer();
    }

    bool has_index_buffer() const {
        return index_buffer.has_value();
    }

    std::optional<VkBuffer> get_index_buffer() const {
        assert(index_buffer.has_value());
        return index_buffer.value().get_buffer();
    }

    const std::uint32_t get_vertex_count() const {
        return number_of_vertices;
    }

    const std::uint32_t get_index_cound() const {
        return number_of_indices;
    }

    // TODO: Update data!
    // void update_vertex_buffer();
    // void update_index_buffer();

    ~MeshBuffer();
};

} // namespace inexor::vulkan_renderer
