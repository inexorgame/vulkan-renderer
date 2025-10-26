#include <gtest/gtest.h>

#include "inexor/vulkan-renderer/tools/queue_selection.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

namespace inexor::vulkan_renderer::tools {

using namespace inexor::vulkan_renderer::wrapper;

TEST(QueueSelection, TheoreticalTest1) {
    const QueueFamilyIndexCandidates expected_result = {
        std::nullopt, // graphics
        std::nullopt, // compute
        std::nullopt, // transfer
        std::nullopt, // sparse binding
        {},           // no queues to create
    };
    const auto actual_result = determine_queue_family_indices({});
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest2) {
    // In this theoretical test, we have only one queue family for graphics.
    // This queue family needs to be chosen by the code.
    // The other queue families remain std::nullopt.
    const QueueFamilyIndexCandidates expected_result = {
        0,            // graphics
        std::nullopt, // compute
        std::nullopt, // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT, // index 0
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest3) {
    // In this theoretical test, we have two queue families for graphics.
    // The code should select the one with index 0. Index 1 would technically also be valid to use, but the iteration
    // through the loop will start at index 0.
    const QueueFamilyIndexCandidates expected_result = {
        0,            // graphics
        std::nullopt, // compute
        std::nullopt, // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT, // index 1
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest4) {
    // In this theoretical test, we have one queue family for graphics and compute at index 0 and a distinct queue
    // family for compute at index 1.
    const QueueFamilyIndexCandidates expected_result = {
        0,            // graphics
        1,            // compute
        std::nullopt, // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT, // index 1
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest5) {
    // In this theoretical test, we have one distinct queue family for compute at index 0 and a queue family for
    // graphics and compute at index 1.
    const QueueFamilyIndexCandidates expected_result = {
        1,            // graphics
        0,            // compute
        std::nullopt, // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1, // Note that this must be index 1 here.
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0, // Note that this must be index 0 here.
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            // This is an unusual test example, because in almost all cases the graphics queue will be at index 0.
            .queueFlags = VK_QUEUE_COMPUTE_BIT, // index 0
        },
        {
            // This is an unusual test example, because in almost all cases the graphics queue will be at index 0.
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, // index 1
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest6) {
    // In this theoretical test, we have one queue family for graphics, compute, and transfer at index 0, a distinct
    // queue family for compute at index 1, and a distinct queue family for transfer at index 2.
    const QueueFamilyIndexCandidates expected_result = {
        0,            // graphics
        1,            // compute
        2,            // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT, // index 2
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest7) {
    // In this theoretical test, we have one queue family for graphics, compute, and transfer at index 0, a queue family
    // for graphics and compute at index 1, and a distinct queue family for transfer at index 2.
    // Note that in this example, queue family index 0 will be chosen for compute although queue family index 1 would be
    // "more distinct" than queue family 0.
    // @TODO Refine granularity for queue selection: If queue family types overlap, sort by distinctness.
    const QueueFamilyIndexCandidates expected_result = {
        1,            // graphics
        0,            // compute
        2,            // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            // This will be selected for both graphics and compute although index 1 is "more distinct" for compute.
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, // index 0
        },
        {
            // This will be selected for both graphics and compute although index 1 is "more distinct" for compute.
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT, // index 2
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest8) {
    // In this theoretical test, we have 3 queue families which all have graphics, compute, and transfer.
    const QueueFamilyIndexCandidates expected_result = {
        0,            // graphics
        1,            // compute
        2,            // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, // index 2
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest9) {
    // In this theoretical test, we have 3 distinct queue families.
    const QueueFamilyIndexCandidates expected_result = {
        2,            // graphics
        1,            // compute
        0,            // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT, // index 2
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest10) {
    // In this theoretical test, we have 3 queue families.
    const QueueFamilyIndexCandidates expected_result = {
        0,            // graphics
        1,            // compute
        2,            // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, // index 2
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest11) {
    // In this theoretical test, we have 4 queue families.
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        1, // compute
        2, // transfer
        3, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 3,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT, // index 2
        },
        {
            .queueFlags = VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_COMPUTE_BIT, // index 3
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, TheoreticalTest12) {
    // In this theoretical test, we have 4 queue families.
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        3, // compute
        2, // transfer
        1, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 3,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT, // index 2
        },
        {
            .queueFlags = VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_COMPUTE_BIT, // index 3
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

/// In the following tests, we will be using data about the queue families of various real graphics cards.
/// Please keep in mind that the queue family configuration depends on the operating system and driver!

TEST(QueueSelection, RealTest_NVIDIA_GeForce_RTX_5090) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        2, // compute
        1, // transfer
        5, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 5,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 2
        },
        {
            .queueFlags =
                VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR, // index 3
        },
        {
            .queueFlags =
                VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_VIDEO_ENCODE_BIT_KHR, // index 4
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 5
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_NVIDIA_GeForce_940M) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        0, // compute
        1, // transfer
        1, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 1
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_NVIDIA_GeForce_660M) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        0, // compute
        1, // transfer
        0, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT, // index 1
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_AMD_Radeon_7900_XTX) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        1, // compute
        2, // transfer
        2, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 2
        },
        {
            .queueFlags = VK_QUEUE_VIDEO_ENCODE_BIT_KHR, // index 3
        },
        {
            .queueFlags = VK_QUEUE_VIDEO_DECODE_BIT_KHR, // index 4
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_Intel_ARC_A770) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        1, // compute
        2, // transfer
        2, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 2
        },
        {
            .queueFlags = VK_QUEUE_VIDEO_DECODE_BIT_KHR, // index 3
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_AMD_Radeon_RX_5600_XT) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        1, // compute
        2, // transfer
        2, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 2
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_Intel_Graphics) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        1, // compute
        2, // transfer
        2, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 2
        },
        {
            .queueFlags = VK_QUEUE_VIDEO_DECODE_BIT_KHR, // index 3
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_GooglePixel7) {
    const QueueFamilyIndexCandidates expected_result = {
        0,            // graphics
        0,            // compute
        std::nullopt, // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, // index 0
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_Intel_UHD_Graphics) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        0, // compute
        0, // transfer
        0, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_NVIDIA_GeForce_GTX_1050) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        2, // compute
        1, // transfer
        1, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 2
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_Intel_ARC_A380) {
    const QueueFamilyIndexCandidates expected_result = {
        0,            // graphics
        2,            // compute
        1,            // transfer
        std::nullopt, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 2
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

TEST(QueueSelection, RealTest_NVIDIA_GeForce_MX150) {
    const QueueFamilyIndexCandidates expected_result = {
        0, // graphics
        2, // compute
        1, // transfer
        1, // sparse binding
        {
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 2,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
            make_info<VkDeviceQueueCreateInfo>({
                .queueFamilyIndex = 1,
                .queueCount = 1,
                .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
            }),
        },
    };
    // We only need to fill the queue flags of VkQueueFamilyProperties for testing.
    const std::vector<VkQueueFamilyProperties> given_input = {
        {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT |
                          VK_QUEUE_SPARSE_BINDING_BIT, // index 0
        },
        {
            .queueFlags = VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 1
        },
        {
            .queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT, // index 2
        },
    };
    const auto actual_result = determine_queue_family_indices(given_input);
    EXPECT_EQ(actual_result, expected_result);
}

} // namespace inexor::vulkan_renderer::tools
