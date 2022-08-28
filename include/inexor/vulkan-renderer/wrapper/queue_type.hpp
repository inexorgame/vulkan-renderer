#pragma once

namespace inexor::vulkan_renderer::wrapper {

/// Specifies the queue type to use for command buffer submissions
enum class QueueType {
    GRAPHICS,
    TRANSFER
    // TODO: Support compute and sparse bindung queues
};

} // namespace inexor::vulkan_renderer::wrapper
