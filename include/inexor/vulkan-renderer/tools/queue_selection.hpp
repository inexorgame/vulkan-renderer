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

    // We need this operator for the unit tests.
    bool operator==(const QueueFamilyIndexCandidates &other) const {
        if (graphics != other.graphics)
            return false;
        if (compute != other.compute)
            return false;
        if (transfer != other.transfer)
            return false;
        if (queues_to_create.size() != other.queues_to_create.size())
            return false;
        for (size_t i = 0; i < queues_to_create.size(); ++i) {
            const auto &a = queues_to_create[i];
            const auto &b = other.queues_to_create[i];

            if (a.queueFamilyIndex != b.queueFamilyIndex)
                return false;
            if (a.queueCount != b.queueCount)
                return false;

            for (uint32_t j = 0; j < a.queueCount && j < b.queueCount; ++j) {
                if (a.pQueuePriorities[j] != b.pQueuePriorities[j])
                    return false;
            }
        }
        return true;
    }
};

/// Automatically selects queue family indices for graphics, compute, and transfer.
/// This code also fills the "queues to create" std::vector of QueueFamilyIndexCandidates.
/// @param name The name of the graphics card.
/// @param props The queue family properties of a physical device.
/// @return A filled QueueFamilyIndexCandidates struct with the queue family indices and the vector of queues to create.
/// Each queue family index could be ``std::nullopt``!.
[[nodiscard]] QueueFamilyIndexCandidates
determine_queue_family_indices(const std::vector<VkQueueFamilyProperties> &props, const std::string &name = "");

} // namespace inexor::vulkan_renderer::tools
