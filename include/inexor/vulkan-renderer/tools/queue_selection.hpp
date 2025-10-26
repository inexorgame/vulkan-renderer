#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <set>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::tools {

// We have to specify this, although it is likely not really used by the gpu.
static constexpr float DEFAULT_QUEUE_PRIORITY{1.0f};

/// A struct for determination of best queue family indices depending on flags.
struct QueueFamilyIndexCandidates {
    std::optional<std::uint32_t> graphics;
    std::optional<std::uint32_t> compute;
    std::optional<std::uint32_t> transfer;
    std::optional<std::uint32_t> sparse_binding;

    std::vector<VkDeviceQueueCreateInfo> queues_to_create;

    // We need this operator for the unit tests.
    bool operator==(const QueueFamilyIndexCandidates &other) const {
        if (graphics != other.graphics) {
            return false;
        }
        if (compute != other.compute) {
            return false;
        }
        if (transfer != other.transfer) {
            return false;
        }
        if (queues_to_create.size() != other.queues_to_create.size()) {
            return false;
        }
        for (size_t i = 0; i < queues_to_create.size(); ++i) {
            const auto &a = queues_to_create[i];
            const auto &b = other.queues_to_create[i];
            if (a.queueFamilyIndex != b.queueFamilyIndex) {
                return false;
            }
            if (a.queueCount != b.queueCount) {
                return false;
            }
            for (uint32_t j = 0; j < a.queueCount && j < b.queueCount; ++j) {
                if (a.pQueuePriorities[j] != b.pQueuePriorities[j])
                    return false;
            }
        }
        return true;
    }
};

/// This will determine the queue family indices to use for graphics queue, compute queue, transfer queue, and sparse
/// binding queue. This will also fill the VkDeviceQueueCreateInfo structs (the queues to create).
/// @param props The queue family properties of a physical device.
/// @return The optimal queue family indices, making as much use of various distinct queue family indices as possible.
QueueFamilyIndexCandidates determine_queue_family_indices(const std::vector<VkQueueFamilyProperties> &props);

} // namespace inexor::vulkan_renderer::tools
