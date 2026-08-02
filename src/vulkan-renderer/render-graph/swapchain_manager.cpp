#include "inexor/vulkan-renderer/render-graph/swapchain_manager.hpp"

#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer_builder.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace inexor::vulkan_renderer::render_graph {

using wrapper::commands::CommandBufferBuilder;

SwapchainManager::SwapchainManager(Device &device) : m_device(device) {}

void SwapchainManager::mark_swapchain_cache_dirty() {
    m_swapchain_cache_dirty = true;
}

void SwapchainManager::clear() {
    m_cached_swapchains.clear();
    m_swapchain_cache_dirty = true;
    m_frame_swapchains.clear();
    m_swapchains_imgs_available.clear();
    m_swapchain_rendering_finished.clear();
    m_frame_slot_count = 1;
    m_current_frame_slot = 0;
}

void SwapchainManager::rebuild_swapchain_cache(const std::vector<std::shared_ptr<GraphicsPass>> &graphics_passes) {
    if (!m_swapchain_cache_dirty) {
        return;
    }

    m_cached_swapchains.clear();

    std::unordered_set<VkSwapchainKHR> seen_handles;
    seen_handles.reserve(graphics_passes.size());

    for (const auto &pass : graphics_passes) {
        for (const auto &write : pass->m_swapchain_writes) {
            const auto swapchain = write.first.lock();
            if (!swapchain) {
                continue;
            }

            const auto handle = swapchain->swapchain();
            if (!seen_handles.insert(handle).second) {
                continue;
            }

            m_cached_swapchains.push_back({
                .handle = handle,
                .swapchain = swapchain,
            });
        }
    }

    m_swapchain_cache_dirty = false;
}

void SwapchainManager::collect_frame_swapchains(const std::vector<std::shared_ptr<GraphicsPass>> &graphics_passes) {
    rebuild_swapchain_cache(graphics_passes);

    m_frame_swapchains.clear();
    m_frame_swapchains.reserve(m_cached_swapchains.size());
    for (const auto &entry : m_cached_swapchains) {
        if (const auto swapchain = entry.swapchain.lock()) {
            m_frame_swapchains.push_back(swapchain);
        }
    }
}

bool SwapchainManager::acquire_next_images() {
    m_swapchains_imgs_available.clear();
    m_swapchain_rendering_finished.clear();
    m_swapchains_imgs_available.reserve(m_frame_swapchains.size());
    m_swapchain_rendering_finished.reserve(m_frame_swapchains.size());

    for (const auto &swapchain : m_frame_swapchains) {
        const auto result = swapchain->acquire_next_image();
        if (result != VK_SUCCESS) {
            m_swapchains_imgs_available.clear();
            m_swapchain_rendering_finished.clear();
            return false;
        }

        swapchain->wait_for_current_image_if_in_flight();
        m_swapchains_imgs_available.emplace_back(swapchain->image_available_semaphore());
        m_swapchain_rendering_finished.emplace_back(swapchain->rendering_finished_semaphore());
    }

    return true;
}

void SwapchainManager::synchronize_frame_context() {
    std::size_t frame_slot_count = 1;
    std::size_t current_frame_slot = 0;
    bool first_swapchain = true;

    for (const auto &swapchain : m_frame_swapchains) {
        const auto swapchain_slot_count = static_cast<std::size_t>(swapchain->frame_slot_count());
        const auto swapchain_frame_slot = static_cast<std::size_t>(swapchain->current_frame_slot());

        if (first_swapchain) {
            frame_slot_count = std::max<std::size_t>(1, swapchain_slot_count);
            current_frame_slot = swapchain_frame_slot;
            first_swapchain = false;
            continue;
        }

        if (frame_slot_count != swapchain_slot_count || current_frame_slot != swapchain_frame_slot) {
            throw std::runtime_error("Error: RenderGraph requires all swapchains to share the same frame-slot state!");
        }
    }

    m_frame_slot_count = frame_slot_count;
    m_current_frame_slot = std::min(current_frame_slot, m_frame_slot_count - 1);
}

void SwapchainManager::prepare_swapchains_for_rendering(CommandBufferBuilder &cmd_buf) const {
    for (const auto &swapchain : m_frame_swapchains) {
        swapchain->change_image_layout_to_prepare_for_rendering(cmd_buf);
    }
}

void SwapchainManager::prepare_swapchains_for_presenting(CommandBufferBuilder &cmd_buf) const {
    for (const auto &swapchain : m_frame_swapchains) {
        swapchain->change_image_layout_to_prepare_for_presenting(cmd_buf);
    }
}

void SwapchainManager::mark_frame_swapchains_in_flight(const VkFence fence) const {
    for (const auto &swapchain : m_frame_swapchains) {
        swapchain->mark_current_image_in_flight(fence);
        swapchain->mark_current_frame_slot_in_flight(fence);
    }
}

void SwapchainManager::present(std::span<const VkSemaphore> wait_semaphores) const {
    for (const auto &swapchain : m_frame_swapchains) {
        swapchain->present(wait_semaphores);
    }
}

} // namespace inexor::vulkan_renderer::render_graph