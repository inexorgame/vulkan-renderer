#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

GraphicsPass::GraphicsPass(std::string name,
                           BufferReads buffer_reads,
                           TextureReads texture_reads,
                           TextureWrites texture_writes,
                           std::function<void(const CommandBuffer &)> on_record,
                           std::optional<VkClearValue> clear_values)
    : m_name(std::move(name)), m_buffer_reads(std::move(buffer_reads)), m_texture_reads(std::move(texture_reads)),
      m_texture_writes(std::move(texture_writes)), m_on_record(std::move(on_record)),
      m_clear_values(std::move(clear_values)) {
    // Make sure there is no more than one index buffer (or none)
    bool index_buffer_present = false;
    for (const auto buffer : m_buffer_reads) {
        // Is this buffer resource an index buffer?
        if (buffer.first.lock()->type() == BufferType::INDEX_BUFFER) {
            // Is an index buffer already specified?
            if (index_buffer_present) {
                throw std::runtime_error("Error: More than one index buffer in graphics pass " + m_name + "!");
            }
            // This was the first index buffer we found
            index_buffer_present = true;
        }
    }
}

GraphicsPass::GraphicsPass(GraphicsPass &&other) noexcept {
    m_name = std::move(other.m_name);
    m_clear_values = other.m_clear_values;
    m_on_record = std::move(other.m_on_record);
    m_buffer_reads = std::move(other.m_buffer_reads);
    m_texture_reads = std::move(other.m_texture_reads);
    m_texture_writes = std::move(other.m_texture_writes);
    m_index_buffer = std::exchange(other.m_index_buffer, VK_NULL_HANDLE);
    m_vertex_buffers = std::move(other.m_vertex_buffers);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, nullptr);
    m_descriptor_set = std::exchange(other.m_descriptor_set, VK_NULL_HANDLE);
}

} // namespace inexor::vulkan_renderer::render_graph
