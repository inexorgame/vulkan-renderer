#pragma once

#include <volk.h>

#include <array>
#include <cstddef>
#include <functional>
#include <mutex>
#include <span>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
class CommandBufferBuilder;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::commands {

/// Caches RenderGraph-owned secondary command buffer recording state.
class CommandBufferCache {
private:
    bool m_use_secondary_command_buffers{true};
    struct SecondaryCommandBufferState {
        VkExtent2D cached_render_extent{0, 0};
        std::vector<bool> dirty_by_frame_slot{true};
    };

    core::Device &m_device;
    std::size_t m_frame_slot_count{1};
    std::size_t m_current_frame_slot{0};
    std::vector<VkFence> m_frame_slot_submission_fences{VK_NULL_HANDLE};
    std::mutex m_secondary_command_buffers_mutex;
    std::unordered_map<std::thread::id, std::unordered_map<std::string, SecondaryCommandBufferState>>
        m_secondary_command_buffers;

    SecondaryCommandBufferState &state_for_pass(const std::string &pass_name);

public:
    explicit CommandBufferCache(core::Device &device, bool use_secondary_command_buffers = true);

    void set_frame_context(std::size_t frame_slot_count, std::size_t current_frame_slot,
                           std::span<const VkFence> frame_slot_submission_fences);

    void invalidate_all_secondary_command_buffers();

    [[nodiscard]] bool uses_secondary_command_buffers() const {
        return m_use_secondary_command_buffers;
    }

    [[nodiscard]] VkCommandBuffer
    record_secondary_command_buffer(const std::string &pass_name, VkExtent2D render_extent,
                                    const VkCommandBufferInheritanceInfo &inheritance_info,
                                    std::function<void(CommandBufferBuilder &)> on_record, bool cache_recording = true);

    void execute_secondary_command_buffer(const CommandBuffer &primary_cmd_buf, const std::string &pass_name,
                                          std::array<float, 4> debug_label_color, const VkRenderingInfo &rendering_info,
                                          VkCommandBuffer secondary_cmd_buf) const;

    void record_secondary_command_buffer(const CommandBuffer &primary_cmd_buf, const std::string &pass_name,
                                         std::array<float, 4> debug_label_color, VkExtent2D render_extent,
                                         const VkCommandBufferInheritanceInfo &inheritance_info,
                                         const VkRenderingInfo &rendering_info,
                                         std::function<void(CommandBufferBuilder &)> on_record);
};

} // namespace inexor::vulkan_renderer::wrapper::commands