#include "inexor/vulkan-renderer/tools/queue_selection.hpp"

#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::tools {

QueueFamilyIndexCandidates determine_queue_family_indices(const std::vector<VkQueueFamilyProperties> &props,
                                                          const std::string &gpu_name) {
    // This lambda will help us to search for a queue family which matches our requirements.
    auto find_queue_family_index_if =
        [&](const std::function<bool(std::uint32_t, const VkQueueFamilyProperties &)> &selection_callback)
        -> std::optional<std::uint32_t> {
        for (std::uint32_t index = 0; const auto &queue_family : props) {
            // Invoke the std::function that was specified by the programmer.
            if (selection_callback(index, queue_family)) {
                return index;
            }
            index++;
        }
        return std::nullopt;
    };

    QueueFamilyIndexCandidates return_value;

    // STEP 1: Find a queue family for graphics.
    {
        const auto graphics_candidate =
            find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
                // It makes no sense to search for a distinct queue family which supports only graphics and no other
                // queue type, because - to the best of our knowledge - this practically does not exist on desktop
                // machines.
                return (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u;
            });

        if (!graphics_candidate) {
            // Ww have not found a queue family for graphics. This should be very unlikely.
            if (!gpu_name.empty()) {
                spdlog::error("Could not find any queue family with VK_QUEUE_GRAPHICS_BIT on GPU '{}'", gpu_name);
            }
        } else {
            // We found a queue family for graphics.
            return_value.graphics = graphics_candidate.value();
        }

        if (return_value.graphics) {
            return_value.queues_to_create.push_back(wrapper::make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = return_value.graphics.value(),
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }));
        }
    }

    // STEP 2: Find a queue family for compute.
    {
        auto compute_candidate =
            find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
                // Try to find a queue family for compute but not graphics or transfer!
                return ((props.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u) &&  // is compute
                       ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0u) && // is not graphics
                       ((props.queueFlags & VK_QUEUE_TRANSFER_BIT) == 0u) && // is not transfer
                       // It gets tricky here: We must make sure that the graphics queue family is even valid!
                       (return_value.graphics.has_value() ? (index != return_value.graphics.value()) : true);
            });

        if (!compute_candidate) {
            // We have not found a distinct queue family for compute. This is not unusual.
            if (!gpu_name.empty()) {
                spdlog::warn("Could not find a distinct queue family with VK_QUEUE_COMPUTE_BIT for GPU '{}'!",
                             gpu_name);
            }

            // This means that all available compute queues are of some other queue type as well, for example graphics.
            // In this case, we just try to find any queue which can be used for compute.
            compute_candidate =
                find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
                    // If we can't find a distinct compute queue, just try to find any compute queue.
                    // This means the compute queue family index is overlapping with transfer or graphics.
                    // In this case we could even be more granular and attempt to find a queue family which is only
                    // associated with graphics or only associated with transfer, but not both.
                    // @TODO Refine granularity: If queue family types overlap, sort by flag distinctness.
                    return (props.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u;
                });

            if (!compute_candidate) {
                if (!gpu_name.empty()) {
                    spdlog::error("Could not find any queue family with VK_QUEUE_COMPUTE_BIT on GPU '{}'", gpu_name);
                }
            } else {
                return_value.compute = compute_candidate.value();
            }
        } else {
            // We have found a distinct queue family for compute.
            return_value.compute = compute_candidate.value();
        }

        // At this point, it could be that a distinct compute queue family was found, or a non-distinct compute queue
        // family was found in a second attempt, or no compute queue family was found at all.
        if (return_value.compute) {
            // If a compute queue family weas found that is not distinct, it means that it is of some other type as
            // well. This means the selected queue family for compute could be of type graphics or transfer as well.
            // Vulkan API only allows us to have one distinct entry for every queue family in VkDeviceQueueCreateInfo!
            // We must check if the queue family index for compute is equal to the queue family index for graphics (if
            // not std::nullopt). You might want to take a moment to read and to understand this if statement.
            if (!return_value.graphics || (return_value.graphics.value() != return_value.compute.value())) {
                // If we reach this, we have found a distinct compute queue.
                return_value.queues_to_create.push_back(wrapper::make_info<VkDeviceQueueCreateInfo>({
                    .queueFamilyIndex = return_value.compute.value(),
                    .queueCount = 1,
                    .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
                }));
            }
        }
    }

    // STEP 3: Find a queue family index for transfer.
    {
        auto transfer_candidate =
            find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
                // Try to find a queue family index which supports transfer but is not the graphics queue!
                return ((props.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u) && // is transfer
                       ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0u) && // is not graphics
                       ((props.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0u) &&  // is not compute
                       // It gets tricky here: We must make sure that the graphics queue family is even valid!
                       (return_value.graphics.has_value() ? (index != return_value.graphics.value()) : true) &&
                       // It gets tricky here: We must make sure that the transfer queue family is even valid!
                       (return_value.transfer.has_value() ? (index != return_value.transfer.value()) : true);
            });

        if (!transfer_candidate) {
            // We have not found a distinct queue family for transfer. This is not unusual.
            if (!gpu_name.empty()) {
                spdlog::warn("Could not find a distinct queue family with VK_QUEUE_TRANSFER_BIT on GPU '{}'!",
                             gpu_name);
            }

            // This means that all available transfer queues are of some other queue type as well, for example graphics.
            // In this case, we just try to find any queue which can be used for transfer.
            transfer_candidate =
                find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
                    // @TODO Refine granularity: If queue family types overlap, sort by flag distinctness.
                    return (props.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u;
                });

            if (!transfer_candidate) {
                if (!gpu_name.empty()) {
                    spdlog::error("Could not find any queue with VK_QUEUE_TRANSFER_BIT on GPU '{}'!", gpu_name);
                }
            } else {
                // We found a queue family for graphics.
                return_value.transfer = transfer_candidate.value();
            }
        } else {
            // We have found a distinct queue family for transfer.
            return_value.transfer = transfer_candidate.value();
        }

        // At this point, it could be that a distinct transfer queue was found, or a non-distinct transfer queue was
        // found in a second attempt, or no transfer queue was found at all.
        if (return_value.transfer) {
            // If a transfer queue family was found which is not distinct, it means that it is of some other type as
            // well. This means the selected queue family for transfer could be of type graphics or transfer as well.
            // Vulkan API only allows us to have one distinct entry for every queue family in VkDeviceQueueCreateInfo!
            // We must check if the queue family index for the transfer queue is equal to the queue family index for the
            // graphics queue (if not std::nullopt) and also make check if the queue family index for the transfer queue
            // is equal to the queue family index for the compute queue (if not std::nullopt). You might want to take a
            // moment to read and to understand this if statement.
            if ((!return_value.graphics || (return_value.transfer.value() != return_value.graphics.value())) &&
                (!return_value.compute || (return_value.transfer.value() != return_value.compute.value()))) {
                // If we reach this, we have found a distinct transfer queue.
                return_value.queues_to_create.push_back(wrapper::make_info<VkDeviceQueueCreateInfo>({
                    .queueFamilyIndex = return_value.transfer.value(),
                    .queueCount = 1,
                    .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
                }));
            }
        }
    }

    // @TODO Implement sparse binding queue and further queue types.

    return return_value;
}

} // namespace inexor::vulkan_renderer::tools
