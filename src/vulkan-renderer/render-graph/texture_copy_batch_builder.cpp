#include "inexor/vulkan-renderer/render-graph/texture_copy_batch_builder.hpp"

#include "inexor/vulkan-renderer/tools/make_info.hpp"
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

[[nodiscard]] bool same_subresource(const VkBufferImageCopy &lhs, const VkBufferImageCopy &rhs) {
    return lhs.imageSubresource.aspectMask == rhs.imageSubresource.aspectMask &&
           lhs.imageSubresource.mipLevel == rhs.imageSubresource.mipLevel &&
           lhs.imageSubresource.baseArrayLayer == rhs.imageSubresource.baseArrayLayer &&
           lhs.imageSubresource.layerCount == rhs.imageSubresource.layerCount;
}

[[nodiscard]] bool same_image_geometry(const VkBufferImageCopy &lhs, const VkBufferImageCopy &rhs) {
    return lhs.imageOffset.x == rhs.imageOffset.x && lhs.imageOffset.y == rhs.imageOffset.y &&
           lhs.imageOffset.z == rhs.imageOffset.z && lhs.imageExtent.width == rhs.imageExtent.width &&
           lhs.imageExtent.height == rhs.imageExtent.height && lhs.imageExtent.depth == rhs.imageExtent.depth &&
           lhs.bufferRowLength == rhs.bufferRowLength && lhs.bufferImageHeight == rhs.bufferImageHeight;
}

void sort_buffer_image_copies(std::vector<VkBufferImageCopy> &regions) {
    std::sort(regions.begin(), regions.end(), [](const VkBufferImageCopy &lhs, const VkBufferImageCopy &rhs) {
        if (lhs.bufferOffset != rhs.bufferOffset) {
            return lhs.bufferOffset < rhs.bufferOffset;
        }
        if (lhs.imageOffset.z != rhs.imageOffset.z) {
            return lhs.imageOffset.z < rhs.imageOffset.z;
        }
        if (lhs.imageOffset.y != rhs.imageOffset.y) {
            return lhs.imageOffset.y < rhs.imageOffset.y;
        }
        if (lhs.imageOffset.x != rhs.imageOffset.x) {
            return lhs.imageOffset.x < rhs.imageOffset.x;
        }
        if (lhs.imageExtent.width != rhs.imageExtent.width) {
            return lhs.imageExtent.width < rhs.imageExtent.width;
        }
        if (lhs.imageExtent.height != rhs.imageExtent.height) {
            return lhs.imageExtent.height < rhs.imageExtent.height;
        }
        return lhs.imageExtent.depth < rhs.imageExtent.depth;
    });
}

} // namespace

std::size_t TextureCopyBatchBuilder::BatchKeyHash::operator()(const BatchKey &key) const noexcept {
    std::size_t seed = hash_handle(key.src_buffer);
    seed ^= hash_handle(key.dst_image) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

void TextureCopyBatchBuilder::set_queue_family_ownership_transfer(const bool needs_transfer,
                                                                  const std::uint32_t transfer_family_index,
                                                                  const std::uint32_t graphics_family_index) {
    m_needs_queue_family_ownership_transfer = needs_transfer;
    m_transfer_family_index = needs_transfer ? transfer_family_index : VK_QUEUE_FAMILY_IGNORED;
    m_graphics_family_index = needs_transfer ? graphics_family_index : VK_QUEUE_FAMILY_IGNORED;
}

void TextureCopyBatchBuilder::add(const PendingTextureCopy &copy_request) {
    m_batches[BatchKey{.src_buffer = copy_request.src_buffer, .dst_image = copy_request.dst_image}].copies.push_back(
        copy_request);
}

void TextureCopyBatchBuilder::add(std::span<const PendingTextureCopy> copy_requests) {
    for (const auto &copy_request : copy_requests) {
        add(copy_request);
    }
}

bool TextureCopyBatchBuilder::empty() const {
    return m_batches.empty();
}

void TextureCopyBatchBuilder::flush(
    const wrapper::commands::CommandBuffer &cmd_buf,
    wrapper::synchronization::PipelineBarrierBatchBuilder &post_copy_barriers,
    wrapper::synchronization::PipelineBarrierBatchBuilder &queue_family_acquire_barriers) {
    if (empty()) {
        return;
    }

    for (auto &[key, batch] : m_batches) {
        if (batch.copies.empty()) {
            continue;
        }

        m_scratch_regions.clear();
        m_scratch_regions.reserve(batch.copies.size());
        for (const auto &copy_request : batch.copies) {
            m_scratch_regions.push_back(copy_request.region);
        }

        sort_buffer_image_copies(m_scratch_regions);

        std::vector<VkBufferImageCopy> deduped_regions;
        deduped_regions.reserve(m_scratch_regions.size());
        for (const auto &region : m_scratch_regions) {
            if (!deduped_regions.empty()) {
                const auto &prev = deduped_regions.back();
                if (same_subresource(prev, region) && same_image_geometry(prev, region) &&
                    prev.bufferOffset == region.bufferOffset) {
                    continue;
                }
            }
            deduped_regions.push_back(region);
        }

        if (deduped_regions.empty()) {
            continue;
        }

        cmd_buf.copy_buffer_to_image(key.src_buffer, key.dst_image, deduped_regions);

        for (const auto &copy_request : batch.copies) {
            if (m_needs_queue_family_ownership_transfer) {
                auto release_barrier = copy_request.post_copy_barrier;
                release_barrier.srcQueueFamilyIndex = m_transfer_family_index;
                release_barrier.dstQueueFamilyIndex = m_graphics_family_index;
                release_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
                release_barrier.dstAccessMask = VK_ACCESS_2_NONE;
                post_copy_barriers.add(release_barrier);

                auto acquire_barrier = copy_request.post_copy_barrier;
                acquire_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
                acquire_barrier.srcAccessMask = VK_ACCESS_2_NONE;
                acquire_barrier.srcQueueFamilyIndex = m_transfer_family_index;
                acquire_barrier.dstQueueFamilyIndex = m_graphics_family_index;
                queue_family_acquire_barriers.add(acquire_barrier);
            } else {
                post_copy_barriers.add(copy_request.post_copy_barrier);
            }
        }
    }

    reset();
}

void TextureCopyBatchBuilder::reset() {
    m_batches.clear();
    m_scratch_regions.clear();
}

} // namespace inexor::vulkan_renderer::render_graph