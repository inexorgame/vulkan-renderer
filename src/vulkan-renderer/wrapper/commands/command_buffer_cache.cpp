#include "inexor/vulkan-renderer/wrapper/commands/command_buffer_cache.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"

#include <limits>
#include <span>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::commands {

CommandBufferCache::CommandBufferCache(core::Device &device, const bool use_secondary_command_buffers)
    : m_device(device), m_use_secondary_command_buffers(use_secondary_command_buffers) {}

CommandBufferCache::SecondaryCommandBufferState &CommandBufferCache::state_for_pass(const std::string &pass_name) {
    std::lock_guard lock(m_secondary_command_buffers_mutex);
    auto &state = m_secondary_command_buffers[std::this_thread::get_id()][pass_name];
    if (state.dirty_by_frame_slot.size() != m_frame_slot_count) {
        state.dirty_by_frame_slot.assign(m_frame_slot_count, true);
    }
    return state;
}

void CommandBufferCache::set_frame_context(const std::size_t frame_slot_count, const std::size_t current_frame_slot,
                                           const std::span<const VkFence> frame_slot_submission_fences) {
    std::lock_guard lock(m_secondary_command_buffers_mutex);
    const bool frame_slot_count_changed = m_frame_slot_count != frame_slot_count;
    m_frame_slot_count = frame_slot_count == 0 ? 1 : frame_slot_count;
    m_current_frame_slot = current_frame_slot < m_frame_slot_count ? current_frame_slot : 0;
    m_frame_slot_submission_fences.assign(frame_slot_submission_fences.begin(), frame_slot_submission_fences.end());
    if (m_frame_slot_submission_fences.size() != m_frame_slot_count) {
        m_frame_slot_submission_fences.resize(m_frame_slot_count, VK_NULL_HANDLE);
    }
    if (!frame_slot_count_changed) {
        return;
    }
    for (auto &[_, thread_states] : m_secondary_command_buffers) {
        for (auto &[__, state] : thread_states) {
            state.dirty_by_frame_slot.assign(m_frame_slot_count, true);
        }
    }
}

void CommandBufferCache::invalidate_all_secondary_command_buffers() {
    std::lock_guard lock(m_secondary_command_buffers_mutex);
    for (auto &[_, thread_states] : m_secondary_command_buffers) {
        for (auto &[__, state] : thread_states) {
            if (state.dirty_by_frame_slot.size() != m_frame_slot_count) {
                state.dirty_by_frame_slot.assign(m_frame_slot_count, true);
            } else {
                std::fill(state.dirty_by_frame_slot.begin(), state.dirty_by_frame_slot.end(), true);
            }
        }
    }
}

VkCommandBuffer
CommandBufferCache::record_secondary_command_buffer(const std::string &pass_name, const VkExtent2D render_extent,
                                                    const VkCommandBufferInheritanceInfo &inheritance_info,
                                                    std::function<void(CommandBufferBuilder &)> on_record,
                                                    const bool cache_recording) {
    auto &state = state_for_pass(pass_name);

    const bool extent_changed = state.cached_render_extent.width != render_extent.width ||
                                state.cached_render_extent.height != render_extent.height;
    if (extent_changed) {
        std::fill(state.dirty_by_frame_slot.begin(), state.dirty_by_frame_slot.end(), true);
    }

    const auto slot_index = m_current_frame_slot;
    const auto &secondary_cmd = m_device.request_secondary_command_buffer(
        VK_QUEUE_GRAPHICS_BIT, pass_name + "[slot " + std::to_string(slot_index) + "]|secondary");

    const bool secondary_cmd_dirty = extent_changed || state.dirty_by_frame_slot[slot_index];

    if (!cache_recording || secondary_cmd_dirty) {
        CommandBufferBuilder secondary_builder(secondary_cmd);
        secondary_builder.reset_recording()
            .begin_secondary_command_buffer(inheritance_info, VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)
            .invoke(on_record)
            .end_recording();
        state.cached_render_extent = render_extent;
        state.dirty_by_frame_slot[slot_index] = false;
    }

    return secondary_cmd.command_buffer();
}

void CommandBufferCache::execute_secondary_command_buffer(const CommandBuffer &primary_cmd_buf,
                                                          const std::string &pass_name,
                                                          const std::array<float, 4> debug_label_color,
                                                          const VkRenderingInfo &rendering_info,
                                                          const VkCommandBuffer secondary_cmd_buf) const {
    auto rendering_info_with_secondary = rendering_info;
    rendering_info_with_secondary.flags |= VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;

    CommandBufferBuilder primary_builder(primary_cmd_buf);
    primary_builder.begin_debug_label_region(pass_name, debug_label_color)
        .begin_rendering(rendering_info_with_secondary)
        .execute_secondary_command_buffers(std::span<const VkCommandBuffer>(&secondary_cmd_buf, 1))
        .end_rendering()
        .end_debug_label_region();
}

void CommandBufferCache::record_secondary_command_buffer(
    const CommandBuffer &primary_cmd_buf, const std::string &pass_name, std::array<float, 4> debug_label_color,
    const VkExtent2D render_extent, const VkCommandBufferInheritanceInfo &inheritance_info,
    const VkRenderingInfo &rendering_info, std::function<void(CommandBufferBuilder &)> on_record) {
    if (!m_use_secondary_command_buffers) {
        CommandBufferBuilder primary_builder(primary_cmd_buf);
        primary_builder.begin_debug_label_region(pass_name, debug_label_color)
            .begin_rendering(rendering_info)
            .invoke(on_record)
            .end_rendering()
            .end_debug_label_region();
        return;
    }

    const auto secondary_handle =
        record_secondary_command_buffer(pass_name, render_extent, inheritance_info, std::move(on_record));
    execute_secondary_command_buffer(primary_cmd_buf, pass_name, debug_label_color, rendering_info, secondary_handle);
}

} // namespace inexor::vulkan_renderer::wrapper::commands
