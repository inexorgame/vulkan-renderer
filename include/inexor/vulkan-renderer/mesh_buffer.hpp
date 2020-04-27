#pragma once

#include "inexor/vulkan-renderer/gpu_memory_buffer.hpp"

#include <vma/vma_usage.h>
#include <vulkan/vulkan.h>

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
struct MeshBuffer {
    Buffer vertex_buffer;

    Buffer index_buffer;

    std::uint32_t number_of_vertices = 0;

    std::uint32_t number_of_indices = 0;

    std::string description = "";

    // Don't forget that index buffers are optional!
    bool index_buffer_available = false;
};

} // namespace inexor::vulkan_renderer
