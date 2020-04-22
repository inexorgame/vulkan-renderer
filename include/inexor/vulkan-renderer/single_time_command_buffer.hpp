#pragma once

#include "inexor/vulkan-renderer/debug_marker_manager.hpp"
#include "inexor/vulkan-renderer/error_handling.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <cassert>

namespace inexor::vulkan_renderer {

/// @class CommandBufferRecorder
/// @brief A class for managing the recording of single time command buffers.
/// This will be used by the texture manager to record copy operations of texture data from CPU to GPU.
class SingleTimeCommandBufferRecorder {
public:
    SingleTimeCommandBufferRecorder() = default;

    ~SingleTimeCommandBufferRecorder() = default;

protected:
    bool command_buffer_recorder_initialised = false;

    VkDevice device = VK_NULL_HANDLE;

    std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

    VkCommandBuffer data_transfer_command_buffer = VK_NULL_HANDLE;

    VkCommandPool data_transfer_command_pool = VK_NULL_HANDLE;

    VkQueue data_transfer_queue = VK_NULL_HANDLE;

protected:
    /// @brief Initialises single time command buffer recording.
    /// @param device [in] The Vulkan device.
    /// @param debug_marker_manager [in] A pointer to the debug marker manager instance.
    VkResult init(const VkDevice &device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager, const VkQueue &data_transfer_queue);

    ///
    VkResult start_recording_of_single_time_command_buffer();

    ///
    VkResult end_recording_of_single_time_command_buffer();

    /// @brief Destroys the command pool.
    void destroy_command_pool();
};

} // namespace inexor::vulkan_renderer
