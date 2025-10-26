#include "inexor/vulkan-renderer/tools/queue_selection.hpp"

#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::tools {

QueueFamilyIndexCandidates determine_queue_family_indices(const std::vector<VkQueueFamilyProperties> &props) {
    // A set of queue family indices which are already in use.
    std::set<std::uint32_t> already_used_queue_family_indices;

    // This lambda rates a all available queue family indices with regards to the desired queue flag.
    auto pick_best_queue = [&](const VkQueueFlags desired_flags) -> std::optional<std::uint32_t> {
        // Lambda to count number of 1-bits in an integer.
        // This is used to count the number of set VkQueueFlags in VkQueueFamilyProperties.
        auto popcount = [](uint32_t x) -> std::uint32_t {
            std::uint32_t count = 0;
            while (x) {
                count += x & 1;
                x >>= 1;
            }
            return count;
        };

        constexpr std::uint32_t VERY_HIGH_PENALTY = 1000;
        constexpr std::uint32_t HIGH_PENALTY = 100;

        auto score_queue = [&](const VkQueueFamilyProperties &props, const bool used) -> std::uint32_t {
            // Penalize queues that cannot satisfy the desired flags.
            if ((props.queueFlags & desired_flags) != desired_flags) {
                return VERY_HIGH_PENALTY;
            }
            // Count extra capabilities (we prefer specialized queues).
            // Fewer extra flags means lower score.
            int score = popcount(props.queueFlags & ~desired_flags);
            if (used) {
                // Penalize already used queues.
                score += HIGH_PENALTY;
            }
            return score;
        };

        // Loop through all available queueu families.
        std::vector<std::pair<std::uint32_t, int>> scored_candidates;
        for (std::uint32_t i = 0; i < props.size(); ++i) {
            bool used = already_used_queue_family_indices.contains(i);
            int score = score_queue(props[i], used);
            scored_candidates.push_back({i, score});
        }

        // Sort by queue score.
        std::sort(scored_candidates.begin(), scored_candidates.end(), [](const auto a, const auto b) {
            // Lower score is better.
            return a.second < b.second;
        });

        // If the score is above VERY_HIGH_PENALTY, the queue does not support the required flag.
        if (!scored_candidates.empty() && scored_candidates.front().second < VERY_HIGH_PENALTY) {
            // This is the most optimal queue family index for this flag.
            // This queue family index is as unique as possible (not any of the already existing queue family indices)
            // and it is as distinct as possible (has as few other flags as possible).
            return scored_candidates.front().first;
        }
        // We could not find any queue which is suitable for the desired flag.
        return std::nullopt;
    };

    QueueFamilyIndexCandidates return_value;

    auto pick_queue_family_and_fill_create_info = [&](std::optional<std::uint32_t> &queue_family_index,
                                                      const VkQueueFlags desired_flags) {
        queue_family_index = pick_best_queue(desired_flags);
        // Have we found any queue family index for this flag?
        if (queue_family_index) {
            // We must make sure that the family is unique in the VkDeviceQueueCreateInfo!
            if (!already_used_queue_family_indices.contains(queue_family_index.value())) {
                // If we reach this, the queue family has not yet been added to the VkDeviceQueueCreateInfo.
                return_value.queues_to_create.push_back(wrapper::make_info<VkDeviceQueueCreateInfo>({
                    .queueFamilyIndex = queue_family_index.value(),
                    .queueCount = 1,
                    .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
                }));
                already_used_queue_family_indices.insert(queue_family_index.value());
            }
        }
    };

    // Determine the optimal queueu family indices for the given flags.
    pick_queue_family_and_fill_create_info(return_value.graphics, VK_QUEUE_GRAPHICS_BIT);
    pick_queue_family_and_fill_create_info(return_value.compute, VK_QUEUE_COMPUTE_BIT);
    pick_queue_family_and_fill_create_info(return_value.transfer, VK_QUEUE_TRANSFER_BIT);
    pick_queue_family_and_fill_create_info(return_value.sparse_binding, VK_QUEUE_SPARSE_BINDING_BIT);

    return return_value;
}

} // namespace inexor::vulkan_renderer::tools
