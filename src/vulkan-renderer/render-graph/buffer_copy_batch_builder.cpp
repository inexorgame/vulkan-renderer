#include "inexor/vulkan-renderer/render-graph/buffer_copy_batch_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/pipeline_barrier_batch_builder.hpp"

#include <algorithm>
#include <cstring>
#include <functional>
#include <type_traits>

namespace inexor::vulkan_renderer::render_graph {

namespace {

template <typename T>
std::size_t hash_handle(T handle) {
    static_assert(std::is_trivially_copyable_v<T>);

    std::uintptr_t value = 0;
    std::memcpy(&value, &handle, std::min(sizeof(value), sizeof(handle)));
    return std::hash<std::uintptr_t>{}(value);
}

[[nodiscard]] bool can_merge_buffer_copy_regions(const VkBufferCopy &lhs, const VkBufferCopy &rhs) {
    return lhs.srcOffset + lhs.size == rhs.srcOffset && lhs.dstOffset + lhs.size == rhs.dstOffset;
}

void coalesce_buffer_copy_regions(std::vector<VkBufferCopy> &regions, std::vector<VkBufferCopy> &scratch) {
    if (regions.size() < 2) {
        return;
    }

    std::sort(regions.begin(), regions.end(), [](const VkBufferCopy &lhs, const VkBufferCopy &rhs) {
        if (lhs.srcOffset != rhs.srcOffset) {
            return lhs.srcOffset < rhs.srcOffset;
        }
        if (lhs.dstOffset != rhs.dstOffset) {
            return lhs.dstOffset < rhs.dstOffset;
        }
        return lhs.size < rhs.size;
    });

    scratch.clear();
    scratch.reserve(regions.size());
    for (const auto &region : regions) {
        if (!scratch.empty() && can_merge_buffer_copy_regions(scratch.back(), region)) {
            scratch.back().size += region.size;
            continue;
        }
        scratch.push_back(region);
    }

    regions = scratch;
}

} // namespace

std::size_t BufferCopyBatchBuilder::BatchKeyHash::operator()(const BatchKey &key) const noexcept {
    std::size_t seed = hash_handle(key.src_buffer);
    seed ^= hash_handle(key.dst_buffer) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

void BufferCopyBatchBuilder::set_queue_family_ownership_transfer(const bool needs_transfer,
                                                                 const std::uint32_t transfer_family_index,
                                                                 const std::uint32_t graphics_family_index) {
    m_needs_queue_family_ownership_transfer = needs_transfer;
    m_transfer_family_index = needs_transfer ? transfer_family_index : VK_QUEUE_FAMILY_IGNORED;
    m_graphics_family_index = needs_transfer ? graphics_family_index : VK_QUEUE_FAMILY_IGNORED;
}

void BufferCopyBatchBuilder::add(const PendingBufferCopy &copy_request) {
    auto &batch = m_batches[BatchKey{.src_buffer = copy_request.src_buffer, .dst_buffer = copy_request.dst_buffer}];
    batch.dst_stage_mask |= copy_request.dst_stage_mask;
    batch.dst_access_mask |= copy_request.dst_access_mask;
    batch.min_offset = std::min(batch.min_offset, copy_request.region.dstOffset);
    batch.max_end = std::max(batch.max_end, copy_request.region.dstOffset + copy_request.region.size);
    batch.regions.push_back(copy_request.region);
}

void BufferCopyBatchBuilder::add(std::span<const PendingBufferCopy> copy_requests) {
    for (const auto &copy_request : copy_requests) {
        add(copy_request);
    }
}

bool BufferCopyBatchBuilder::empty() const {
    return m_batches.empty();
}

void BufferCopyBatchBuilder::flush(
    const wrapper::commands::CommandBuffer &cmd_buf,
    wrapper::synchronization::PipelineBarrierBatchBuilder &post_copy_barriers,
    wrapper::synchronization::PipelineBarrierBatchBuilder &queue_family_acquire_barriers) {
    if (empty()) {
        return;
    }

    for (auto &[key, batch] : m_batches) {
        coalesce_buffer_copy_regions(batch.regions, m_scratch_regions);
        if (batch.regions.empty()) {
            continue;
        }

        cmd_buf.copy_buffer(key.src_buffer, key.dst_buffer, batch.regions);

        const auto dst_stage_mask = batch.dst_stage_mask | VK_PIPELINE_STAGE_2_COPY_BIT;
        const auto dst_access_mask = batch.dst_access_mask | VK_ACCESS_2_TRANSFER_WRITE_BIT;
        if (dst_stage_mask == VK_PIPELINE_STAGE_2_NONE || dst_access_mask == VK_ACCESS_2_NONE) {
            continue;
        }

        const auto barrier_size = batch.max_end - batch.min_offset;
        if (m_needs_queue_family_ownership_transfer) {
            post_copy_barriers.add(VkBufferMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
                .dstAccessMask = VK_ACCESS_2_NONE,
                .srcQueueFamilyIndex = m_transfer_family_index,
                .dstQueueFamilyIndex = m_graphics_family_index,
                .buffer = key.dst_buffer,
                .offset = batch.min_offset,
                .size = barrier_size,
            });

            queue_family_acquire_barriers.add(VkBufferMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                .srcAccessMask = VK_ACCESS_2_NONE,
                .dstStageMask = dst_stage_mask,
                .dstAccessMask = dst_access_mask,
                .srcQueueFamilyIndex = m_transfer_family_index,
                .dstQueueFamilyIndex = m_graphics_family_index,
                .buffer = key.dst_buffer,
                .offset = batch.min_offset,
                .size = barrier_size,
            });
        } else {
            post_copy_barriers.add(VkBufferMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask = dst_stage_mask,
                .dstAccessMask = dst_access_mask,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = key.dst_buffer,
                .offset = batch.min_offset,
                .size = barrier_size,
            });
        }
    }

    reset();
}

void BufferCopyBatchBuilder::reset() {
    m_batches.clear();
    m_scratch_regions.clear();
}

} // namespace inexor::vulkan_renderer::render_graph