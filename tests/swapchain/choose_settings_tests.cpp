#include "inexor/vulkan-renderer/wrapper/swapchain/swapchain_utils.hpp"

#include <gtest/gtest.h>
#include <spdlog/sinks/null_sink.h>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::swapchain {

void silence_spdlog() {
    auto null_logger = std::make_shared<spdlog::logger>("null", std::make_shared<spdlog::sinks::null_sink_mt>());
    spdlog::set_default_logger(null_logger);
}

TEST(Swapchain, choose_array_layers) {
    silence_spdlog();
    VkSurfaceCapabilitiesKHR caps{};
    caps.maxImageArrayLayers = 3;
    // Test if clamping between 1 and maxImageArrayLayers works.
    EXPECT_EQ(choose_array_layers(caps, 0), 1);
    EXPECT_EQ(choose_array_layers(caps, 1), 1);
    EXPECT_EQ(choose_array_layers(caps, 2), 2);
    EXPECT_EQ(choose_array_layers(caps, 3), 3);
    EXPECT_EQ(choose_array_layers(caps, 4), 3);
    EXPECT_EQ(choose_array_layers(caps, 5), 3);
}

TEST(Swapchain, choose_composite_alpha) {
    silence_spdlog();
    static constexpr std::array<VkCompositeAlphaFlagBitsKHR, 4> composite_alpha_flags{
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    // Combine all composite alpha flag bits into one bitmask.
    VkCompositeAlphaFlagsKHR supported_flags{};
    for (const auto &requested_flag : composite_alpha_flags) {
        supported_flags |= requested_flag;
    }
    for (const auto &requested_flag : composite_alpha_flags) {
        // For each composite alpha flag, test if an exception is thrown if no supported alpha flags are specified.
        EXPECT_ANY_THROW(const auto &result = choose_composite_alpha(0, requested_flag));
        // If we iterate through all supported flags and request a specific one, that specific one must be chosen.
        EXPECT_EQ(choose_composite_alpha(supported_flags, requested_flag), requested_flag);
    }
}

TEST(Swapchain, choose_image_count) {
    silence_spdlog();
    VkSurfaceCapabilitiesKHR caps;
    caps.minImageCount = 1;
    caps.maxImageCount = 3;
    EXPECT_EQ(choose_image_count(caps, 1), 2);
    EXPECT_EQ(choose_image_count(caps, 2), 3);
    EXPECT_EQ(choose_image_count(caps, 3), 3);
}

TEST(Swapchain, choose_image_extent) {
    silence_spdlog();
    VkSurfaceCapabilitiesKHR caps;
    caps.maxImageExtent = {1920, 1080};
    caps.minImageExtent = {128, 238};
    const VkExtent2D current_extent{512, 512};
    const VkExtent2D numeric_limit{std::numeric_limits<std::uint32_t>::max(),
                                   std::numeric_limits<std::uint32_t>::max()};
    // If the current extent is numeric max, the requested extent is returned
    const auto chosen_extent1 = choose_image_extent(numeric_limit, caps, current_extent);
    EXPECT_EQ(chosen_extent1.width, current_extent.width);
    EXPECT_EQ(chosen_extent1.height, current_extent.height);
    // The application can select a dimension withing the range limits of the surface.
    const VkExtent2D requested_1{2080, 4096};
    const VkExtent2D current_extent_invalid{0, 0};
    const auto chosen_extent2 = choose_image_extent(requested_1, caps, current_extent_invalid);
    EXPECT_EQ(chosen_extent2.width, caps.maxImageExtent.width);
    EXPECT_EQ(chosen_extent2.height, caps.maxImageExtent.height);
}

TEST(Swapchain, choose_image_usage) {
    silence_spdlog();
    // An exception must be thrown if no image usage flags are supported but I request any.
    EXPECT_ANY_THROW(const auto result = choose_image_usage(0, 0, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is requested and supported and will be chosen automatically.
    EXPECT_EQ(choose_image_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    // None specified but VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT is supported and will be chosen automatically.
    EXPECT_EQ(choose_image_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, 0),
              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
}

TEST(Swapchain, choose_present_mode) {
    silence_spdlog();
    std::vector<VkPresentModeKHR> available_present_modes{};
    // An exception must be thrown if no present modes are available.
    EXPECT_ANY_THROW(const auto &result = choose_present_mode(available_present_modes, false));
    // Add VK_PRESENT_MODE_FIFO_RELAXED_KHR to the list of available present modes.
    // From this point on, VK_PRESENT_MODE_FIFO_RELAXED_KHR must be chosen at all time if vsync is enabled.
    available_present_modes.push_back(VK_PRESENT_MODE_FIFO_KHR);
    available_present_modes.push_back(VK_PRESENT_MODE_FIFO_RELAXED_KHR);
    EXPECT_EQ(choose_present_mode(available_present_modes, false), VK_PRESENT_MODE_FIFO_RELAXED_KHR);
    EXPECT_EQ(choose_present_mode(available_present_modes, true), VK_PRESENT_MODE_FIFO_KHR);

    available_present_modes.push_back(VK_PRESENT_MODE_MAILBOX_KHR);
    EXPECT_EQ(choose_present_mode(available_present_modes, false), VK_PRESENT_MODE_MAILBOX_KHR);
    EXPECT_EQ(choose_present_mode(available_present_modes, true), VK_PRESENT_MODE_FIFO_KHR);

    available_present_modes.push_back(VK_PRESENT_MODE_IMMEDIATE_KHR);
    EXPECT_EQ(choose_present_mode(available_present_modes, false), VK_PRESENT_MODE_IMMEDIATE_KHR);
    EXPECT_EQ(choose_present_mode(available_present_modes, true), VK_PRESENT_MODE_FIFO_KHR);
}

TEST(Swapchain, choose_surface_format) {
    silence_spdlog();
    const std::vector<VkSurfaceFormatKHR> priority_list1{
        {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
    };
    const std::vector<VkSurfaceFormatKHR> priority_list2{
        {VK_FORMAT_R4G4_UNORM_PACK8, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT},
        {VK_FORMAT_R8_SNORM, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT},
    };
    const auto available_surface_formats = priority_list1;

    const VkSurfaceFormatKHR format1{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    auto result = choose_surface_format(available_surface_formats, priority_list1);
    EXPECT_EQ(result.format, format1.format);
    EXPECT_EQ(result.colorSpace, format1.colorSpace);

    EXPECT_ANY_THROW(choose_surface_format(priority_list2, priority_list1));
}

TEST(Swapchain, choose_transform) {
    silence_spdlog();
    VkSurfaceCapabilitiesKHR caps;
    caps.supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    EXPECT_EQ(choose_transform(caps, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR), VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
}

} // namespace inexor::vulkan_renderer::wrapper::swapchain
