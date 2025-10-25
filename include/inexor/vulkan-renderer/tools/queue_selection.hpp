#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::tools {

// We have to specify this, although it is likely not really used by the gpu.
static constexpr float DEFAULT_QUEUE_PRIORITY{1.0f};

///
struct QueueFamilyIndexCandidates {
    std::optional<std::uint32_t> graphics;
    std::optional<std::uint32_t> compute;
    std::optional<std::uint32_t> transfer;

    std::vector<VkDeviceQueueCreateInfo> queues_to_create;
};

///
/// @param name The name of the graphics card.
/// @param props The queue family properties of a physical device.
/// @exception std::runtime_error If no queue with VK_QUEUE_GRAPHICS_BIT could be found.
/// @return
[[nodiscard]] QueueFamilyIndexCandidates
determine_queue_family_indices(const std::vector<VkQueueFamilyProperties> &props, std::string name = "");

} // namespace inexor::vulkan_renderer::tools
