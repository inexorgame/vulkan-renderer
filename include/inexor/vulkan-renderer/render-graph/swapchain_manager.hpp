#pragma once

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchains/swapchain.hpp"

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace inexor::vulkan_renderer::render_graph {

class GraphicsPass;

using wrapper::commands::CommandBuffer;
using wrapper::core::Device;
using wrapper::swapchains::Swapchain;

class SwapchainManager {
private:
    struct SwapchainCacheEntry {
        VkSwapchainKHR handle{VK_NULL_HANDLE};
        std::weak_ptr<Swapchain> swapchain;
    };

    Device &m_device;
    std::vector<SwapchainCacheEntry> m_cached_swapchains;
    bool m_swapchain_cache_dirty{true};
    std::vector<std::shared_ptr<Swapchain>> m_frame_swapchains;
    std::vector<VkSemaphore> m_swapchains_imgs_available;
    std::vector<VkSemaphore> m_swapchain_rendering_finished;
    std::size_t m_frame_slot_count{1};
    std::size_t m_current_frame_slot{0};

    void rebuild_swapchain_cache(const std::vector<std::shared_ptr<GraphicsPass>> &graphics_passes);

public:
    explicit SwapchainManager(Device &device);

    void mark_swapchain_cache_dirty();

    void clear();

    void collect_frame_swapchains(const std::vector<std::shared_ptr<GraphicsPass>> &graphics_passes);

    [[nodiscard]] bool acquire_next_images();

    void synchronize_frame_context();

    [[nodiscard]] const std::vector<VkSemaphore> &image_available_semaphores() const {
        return m_swapchains_imgs_available;
    }

    [[nodiscard]] const std::vector<VkSemaphore> &rendering_finished_semaphores() const {
        return m_swapchain_rendering_finished;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<Swapchain>> &frame_swapchains() const {
        return m_frame_swapchains;
    }

    void prepare_swapchains_for_rendering(const CommandBuffer &cmd_buf) const;

    void prepare_swapchains_for_presenting(const CommandBuffer &cmd_buf) const;

    void mark_frame_swapchains_in_flight(VkFence fence) const;

    void present(std::span<const VkSemaphore> wait_semaphores) const;

    [[nodiscard]] std::size_t frame_slot_count() const {
        return m_frame_slot_count;
    }

    [[nodiscard]] std::size_t current_frame_slot() const {
        return m_current_frame_slot;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
