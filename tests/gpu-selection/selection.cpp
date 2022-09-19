#include <gtest/gtest.h>

#include <inexor/vulkan-renderer/wrapper/device.hpp>

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

const VkPhysicalDevice device1{reinterpret_cast<VkPhysicalDevice>(0x1)}; // NOLINT
const VkPhysicalDevice device2{reinterpret_cast<VkPhysicalDevice>(0x2)}; // NOLINT

TEST(GpuSelection, PhysicalDeviceTypeTest) {
    const DeviceInfo gpu1{
        .name = "Discrete GPU",
        .physical_device = device1,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = {},
        .extensions = {},
    };
    const DeviceInfo gpu2{
        .name = "Integrated GPU",
        .physical_device = device2,
        .type = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, // this will make gpu1 preferred
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = {},
        .extensions = {},
    };

    std::vector physical_devices{gpu1, gpu2};
    EXPECT_EQ(Device::pick_best_physical_device(std::move(physical_devices), {}, {}), device1);
}

TEST(GpuSelection, DeviceLocalMemoryTest) {
    const DeviceInfo gpu1{
        .name = "Discrete GPU 1 with big memory",
        .physical_device = device1,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = {},
        .extensions = {},
    };
    const DeviceInfo gpu2{
        .name = "Discrete GPU 1 with small memory",
        .physical_device = device2,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 128, // this will make gpu1 preferred
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = {},
        .extensions = {},
    };

    std::vector physical_devices{gpu1, gpu2};
    EXPECT_EQ(Device::pick_best_physical_device(std::move(physical_devices), {}, {}), device1);
}

TEST(GpuSelection, SwapchainTest) {
    const DeviceInfo gpu1{
        .name = "GPU 1 with swapchain",
        .physical_device = device1,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = {},
        .extensions = {},
    };
    const DeviceInfo gpu2{
        .name = "GPU 2 without swapchain",
        .physical_device = device2,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = false, // this will make gpu1 preferred
        .features = {},
        .extensions = {},
    };

    std::vector physical_devices{gpu1, gpu2};
    EXPECT_EQ(Device::pick_best_physical_device(std::move(physical_devices), {}, {}), device1);
}

TEST(GpuSelection, PresentationTest) {
    const DeviceInfo gpu1{
        .name = "GPU 1 with presentation support",
        .physical_device = device1,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = {},
        .extensions = {},
    };
    const DeviceInfo gpu2{
        .name = "GPU 2 without presentation support",
        .physical_device = device2,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = false, // this will make gpu1 preferred
        .swapchain_supported = true,
        .features = {},
        .extensions = {},
    };

    std::vector physical_devices{gpu1, gpu2};
    EXPECT_EQ(Device::pick_best_physical_device(std::move(physical_devices), {}, {}), device1);
}

TEST(GpuSelection, DeviceFeatureTest) {
    const auto FEATURE_COUNT = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
    const std::vector<VkBool32> all_features_enabled(FEATURE_COUNT, VK_TRUE);
    const std::vector<VkBool32> all_features_disabled(FEATURE_COUNT, VK_FALSE);
    VkPhysicalDeviceFeatures all_features_enabled2{};
    VkPhysicalDeviceFeatures all_features_disabled2{};
    std::memcpy(&all_features_enabled2, all_features_enabled.data(), sizeof(VkPhysicalDeviceFeatures));
    std::memcpy(&all_features_disabled2, all_features_disabled.data(), sizeof(VkPhysicalDeviceFeatures));

    const DeviceInfo gpu1{
        .name = "GPU 1 with all the features",
        .physical_device = device1,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = all_features_enabled2,
        .extensions = {},
    };
    const DeviceInfo gpu2{
        .name = "GPU 2 with no features",
        .physical_device = device2,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = all_features_disabled2, // this will make gpu1 preferred
        .extensions = {},
    };

    // Because gpu 1 has all features enabled and gpu 2 has all features disabled, gpu 1 must always be preferred
    for (std::size_t i = 0; i < FEATURE_COUNT; i++) {
        std::vector<VkBool32> one_feature_enabled(FEATURE_COUNT, VK_FALSE);
        one_feature_enabled[i] = VK_TRUE;
        VkPhysicalDeviceFeatures one_feature_enabled2{};
        std::memcpy(&one_feature_enabled2, one_feature_enabled.data(), sizeof(VkPhysicalDeviceFeatures));
        std::vector physical_devices{gpu1, gpu2};
        EXPECT_EQ(Device::pick_best_physical_device(std::move(physical_devices), one_feature_enabled2, {}), device1);
    }
}

TEST(GpuSelection, DeviceExtensionTest) {
    const VkExtensionProperties extension{.extensionName{VK_EXT_DEBUG_MARKER_EXTENSION_NAME}};
    const std::vector<VkExtensionProperties> extensions{extension};
    const DeviceInfo gpu1{
        .name = "GPU 1 with debug marker extension",
        .physical_device = device1,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = {},
        .extensions = extensions,
    };
    const DeviceInfo gpu2{
        .name = "GPU 2 without debug marker extension",
        .physical_device = device2,
        .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        .total_device_local = 1024,
        .presentation_supported = true,
        .swapchain_supported = true,
        .features = {},
        .extensions = {}, // this will make gpu1 preferred
    };

    std::vector physical_devices{gpu1, gpu2};
    std::vector<const char *> required_extension{VK_EXT_DEBUG_MARKER_EXTENSION_NAME};
    EXPECT_EQ(Device::pick_best_physical_device(std::move(physical_devices), {}, required_extension), device1);
}

} // namespace inexor::vulkan_renderer::wrapper
