#include <gtest/gtest.h>

#include <inexor/vulkan-renderer/wrapper/device.hpp>

namespace inexor::vulkan_renderer::wrapper {

TEST(PhysicalDeviceRating, DisqualifyNoSwapchainSupport) {
    // If a physical device does not support swapchains, it's unsuitable
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, {}, false, true), -1);
}

TEST(PhysicalDeviceRating, DisqualifyNoPresentationSupport) {
    // If a physical device does not support presentation, it's unsuitable
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, {}, false, true), -1);
}

TEST(PhysicalDeviceRating, RateMiscPhysicalDeviceTypes) {
    // Without specifying any memory properties, physical devices which are not discrete gpus or integrated gpus should
    // get a score of 1 only
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_OTHER, {}, true, true), 1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, {}, true, true), 1);
    EXPECT_EQ(rate_physical_device(VK_PHYSICAL_DEVICE_TYPE_CPU, {}, true, true), 1);
}

} // namespace inexor::vulkan_renderer::wrapper
