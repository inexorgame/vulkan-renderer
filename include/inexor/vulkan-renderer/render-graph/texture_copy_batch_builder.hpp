#pragma once

#include "inexor/vulkan-renderer/render-graph/texture.hpp"

#include <volk.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <unordered_map>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::commands {
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::synchronization {
class PipelineBarrierBatchBuilder;
} // namespace inexor::vulkan_renderer::wrapper::synchronization

namespace inexor::vulkan_renderer::render_graph {

class TextureCopyBatchBuilder {
private:
    struct BatchKey {
        VkBuffer src_buffer{VK_NULL_HANDLE};
        VkImage dst_image{VK_NULL_HANDLE};

        bool operator==(const BatchKey &) const = default;
    };

    struct BatchKeyHash {
        std::size_t operator()(const BatchKey &key) const noexcept;
    };

    struct BatchValue {
        std::vector<PendingTextureCopy> copies;
    };

    std::unordered_map<BatchKey, BatchValue, BatchKeyHash> m_batches;
    std::vector<VkBufferImageCopy> m_scratch_regions;
    bool m_needs_queue_family_ownership_transfer{false};
    std::uint32_t m_transfer_family_index{VK_QUEUE_FAMILY_IGNORED};
    std::uint32_t m_graphics_family_index{VK_QUEUE_FAMILY_IGNORED};

public:
    TextureCopyBatchBuilder() = default;

    void set_queue_family_ownership_transfer(bool needs_transfer, std::uint32_t transfer_family_index,
                                             std::uint32_t graphics_family_index);

    void add(const PendingTextureCopy &copy_request);
    void add(std::span<const PendingTextureCopy> copy_requests);

    [[nodiscard]] bool empty() const;

    void flush(const wrapper::commands::CommandBuffer &cmd_buf,
               wrapper::synchronization::PipelineBarrierBatchBuilder &post_copy_barriers,
               wrapper::synchronization::PipelineBarrierBatchBuilder &queue_family_acquire_barriers);

    void reset();
};

} // namespace inexor::vulkan_renderer::render_graph