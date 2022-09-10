#include <gtest/gtest.h>

#include <inexor/vulkan-renderer/wrapper/device.hpp>

namespace inexor::vulkan_renderer::wrapper {

TEST(PhysicalDeviceRating, DisqualifyNoSwapchainSupport) {
    // If a physical device does not support swapchains, it's unsuitable
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_OTHER, {}, false, true), -1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, {}, false, true), -1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, {}, false, true), -1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, {}, false, true), -1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_CPU, {}, false, true), -1);
}

TEST(PhysicalDeviceRating, DisqualifyNoPresentationSupport) {
    // If a physical device does not support presentation, it's unsuitable
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_OTHER, {}, false, true), -1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, {}, false, true), -1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, {}, false, true), -1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, {}, false, true), -1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_CPU, {}, false, true), -1);
}

TEST(PhysicalDeviceRating, RateMiscPhysicalDeviceTypes) {
    // Without specifying any memory properties, physical devices which are not discrete gpus or integrated gpus should
    // get a score of 1 only
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_OTHER, {}, true, true), 1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, {}, true, true), 1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_CPU, {}, true, true), 1);
}

TEST(PhysicalDeviceRating, Scenario1) {
    // In this scenario, we have 2 physical devices

    // NVIDIA GeForce RTX 2060
    const auto type1{VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU};
    const VkPhysicalDeviceMemoryProperties mem_props1{
        .memoryHeapCount = 3,
        .memoryHeaps{
            {.size = 6270484480, .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
            {.size = 8482541568, .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
            {.size = 224395264, .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
        },
    };

    // Intel HD Graphics 5000
    const auto type2{VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};
    const VkPhysicalDeviceMemoryProperties mem_props2{
        .memoryHeapCount = 1,
        .memoryHeaps{
            {.size = 8589934592, .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
        },
    };

    const auto rating1 = rate_physical_device(type1, mem_props1, true, true);
    const auto rating2 = rate_physical_device(type2, mem_props2, true, true);
    EXPECT_TRUE(rating1 > rating2);
}

TEST(PhysicalDeviceRating, Scenario2) {
    // In this scenario, we have 3 physical devices

    // llvmpipe (LLVM 14.0.6, 256 bits)
    const auto type1{VK_PHYSICAL_DEVICE_TYPE_CPU};
    const VkPhysicalDeviceMemoryProperties mem_props1{
        .memoryHeapCount = 1,
        .memoryHeaps{
            {.size = 2147483648, .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
        },
    };

    // AMD Radeon RX 6800 XT
    const auto type2{VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU};
    const VkPhysicalDeviceMemoryProperties mem_props2{
        .memoryHeapCount = 1,
        // For simplicity, we only mark the device local memory as such with the flags
        // If a memory is not device local, the flag will simply be set to 0
        .memoryHeaps{
            {.size = 16876830720, .flags = 0},
            {.size = 17163091968, .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
        },
    };

    // NVIDIA GeForce RTX 3080
    const auto type3{VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU};
    const VkPhysicalDeviceMemoryProperties mem_props3{
        .memoryHeapCount = 1,
        // For simplicity, we only mark the device local memory as such with the flags
        // If a memory is not device local, the flag will simply be set to 0
        .memoryHeaps{
            {.size = 10566500352, .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
            {.size = 34312810496, .flags = 0},
            {.size = 224395264, .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
        },
    };

    const auto rating1 = rate_physical_device(type1, mem_props1, true, true);
    const auto rating2 = rate_physical_device(type2, mem_props2, true, true);
    const auto rating3 = rate_physical_device(type3, mem_props3, true, true);
    EXPECT_TRUE(rating1 < rating2);
    EXPECT_TRUE(rating2 < rating3);
}

} // namespace inexor::vulkan_renderer::wrapper
