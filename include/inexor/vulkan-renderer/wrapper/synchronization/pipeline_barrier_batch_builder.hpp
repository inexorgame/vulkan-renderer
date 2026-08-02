#pragma once

#include <volk.h>

#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::commands {
/// Forward declaration
class CommandBufferBuilder;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::synchronization {

/// A builder pattern for Vulkan pipeline barriers.
/// Collects barriers and submits them in a single vkCmdPipelineBarrier2 call on flush.
class PipelineBarrierBatchBuilder {
private:
    std::vector<VkMemoryBarrier2> m_memory_barriers;
    std::vector<VkBufferMemoryBarrier2> m_buffer_barriers;
    std::vector<VkImageMemoryBarrier2> m_image_barriers;

public:
    PipelineBarrierBatchBuilder() = default;

    template <typename BarrierType>
    auto &add(const BarrierType &barrier) {
        // Put the barrier into the correct vector depending on its type
        using U = std::remove_cvref_t<BarrierType>;
        if constexpr (std::is_same_v<U, VkMemoryBarrier2>) {
            m_memory_barriers.push_back(barrier);
        } else if constexpr (std::is_same_v<U, VkBufferMemoryBarrier2>) {
            m_buffer_barriers.push_back(barrier);
        } else if constexpr (std::is_same_v<U, VkImageMemoryBarrier2>) {
            m_image_barriers.push_back(barrier);
        } else {
            throw std::invalid_argument("Unsupported Vulkan barrier type");
        }
        return *this;
    }

    template <typename BarrierType>
    auto &add(std::span<const BarrierType> barriers) {
        // Put the barriers into the correct vector depending on its type
        using U = std::remove_cv_t<BarrierType>;
        if constexpr (std::is_same_v<U, VkMemoryBarrier2>) {
            m_memory_barriers.insert(m_memory_barriers.end(), barriers.begin(), barriers.end());
        } else if constexpr (std::is_same_v<U, VkBufferMemoryBarrier2>) {
            m_buffer_barriers.insert(m_buffer_barriers.end(), barriers.begin(), barriers.end());
        } else if constexpr (std::is_same_v<U, VkImageMemoryBarrier2>) {
            m_image_barriers.insert(m_image_barriers.end(), barriers.begin(), barriers.end());
        } else {
            throw std::invalid_argument("Unsupported Vulkan barrier type");
        }
        return *this;
    }

    [[nodiscard]] bool empty() const;

    /// Flushes only when at least one barrier has been queued.
    void flush_if_not_empty(wrapper::commands::CommandBufferBuilder &cmd_buf);

    /// Flush queued barriers. Throws if called while empty.
    void flush(wrapper::commands::CommandBufferBuilder &cmd_buf);
    void reset();
};

} // namespace inexor::vulkan_renderer::wrapper::synchronization
