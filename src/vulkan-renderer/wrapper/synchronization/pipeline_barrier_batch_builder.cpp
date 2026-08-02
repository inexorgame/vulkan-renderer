#include "inexor/vulkan-renderer/wrapper/synchronization/pipeline_barrier_batch_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::synchronization {

bool PipelineBarrierBatchBuilder::empty() const {
    return m_memory_barriers.empty() && m_buffer_barriers.empty() && m_image_barriers.empty();
}

void PipelineBarrierBatchBuilder::flush(const wrapper::commands::CommandBuffer &cmd_buf) {
    if (empty()) {
        throw std::logic_error("PipelineBarrierBatchBuilder::flush called with no barriers. Use flush_if_not_empty() "
                               "if this is expected.");
    }

    const auto dependency_info = tools::make_info<VkDependencyInfo>({
        .memoryBarrierCount = static_cast<std::uint32_t>(m_memory_barriers.size()),
        .pMemoryBarriers = m_memory_barriers.empty() ? nullptr : m_memory_barriers.data(),
        .bufferMemoryBarrierCount = static_cast<std::uint32_t>(m_buffer_barriers.size()),
        .pBufferMemoryBarriers = m_buffer_barriers.empty() ? nullptr : m_buffer_barriers.data(),
        .imageMemoryBarrierCount = static_cast<std::uint32_t>(m_image_barriers.size()),
        .pImageMemoryBarriers = m_image_barriers.empty() ? nullptr : m_image_barriers.data(),
    });

    vkCmdPipelineBarrier2(cmd_buf.command_buffer(), &dependency_info);
    reset();
}

void PipelineBarrierBatchBuilder::flush_if_not_empty(const wrapper::commands::CommandBuffer &cmd_buf) {
    if (!empty()) {
        flush(cmd_buf);
    }
}

void PipelineBarrierBatchBuilder::reset() {
    m_memory_barriers.clear();
    m_buffer_barriers.clear();
    m_image_barriers.clear();
}

} // namespace inexor::vulkan_renderer::wrapper::synchronization
