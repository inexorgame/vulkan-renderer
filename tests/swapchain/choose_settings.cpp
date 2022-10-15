#include <inexor/vulkan-renderer/wrapper/swapchain.hpp>

#include <gtest/gtest.h>

#include <vector>

namespace inexor::vulkan_renderer::wrapper {

TEST(Swapchain, choose_composite_alpha) {
    const std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags{
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR};

    VkCompositeAlphaFlagsKHR supported_flags{};
    for (const auto flag : composite_alpha_flags) {
        supported_flags |= flag;
    }

    for (const auto flag : composite_alpha_flags) {
        EXPECT_EQ(Swapchain::choose_composite_alpha(flag, supported_flags), flag);
        EXPECT_EQ(Swapchain::choose_composite_alpha(flag, 0x0), std::nullopt);
    }
}

TEST(Swapchain, choose_image_extent) {
    const VkExtent2D numeric_limit{std::numeric_limits<std::uint32_t>::max(),
                                   std::numeric_limits<std::uint32_t>::max()};
    const VkExtent2D invalid_extent{0, 0};
    const VkExtent2D min_extent{128, 128};
    const VkExtent2D max_extent{1024, 1024};
    const VkExtent2D current_extent{512, 512};
    const VkExtent2D request1{64, 64};
    const VkExtent2D request2{256, 256};
    const VkExtent2D request3{1024, 1024};
    const VkExtent2D request4{2048, 2048};

    // If the current extent is numeric max, the requested extent is returned
    auto result = Swapchain::choose_image_extent(request1, min_extent, max_extent, numeric_limit);
    EXPECT_EQ(result.width, request1.width);
    EXPECT_EQ(result.height, request1.height);

    // If width or height of the requested extent is 0, the current extent is returned
    result = Swapchain::choose_image_extent(invalid_extent, min_extent, max_extent, current_extent);
    EXPECT_EQ(result.width, current_extent.width);
    EXPECT_EQ(result.height, current_extent.height);

    // Test clamp to min extent
    result = Swapchain::choose_image_extent(request1, min_extent, max_extent, current_extent);
    EXPECT_EQ(result.width, min_extent.width);
    EXPECT_EQ(result.height, min_extent.height);

    result = Swapchain::choose_image_extent(request2, min_extent, max_extent, current_extent);
    EXPECT_EQ(result.width, request2.width);
    EXPECT_EQ(result.height, request2.height);

    result = Swapchain::choose_image_extent(request3, min_extent, max_extent, current_extent);
    EXPECT_EQ(result.width, request3.width);
    EXPECT_EQ(result.height, request3.height);

    // Test clamp to max extent
    result = Swapchain::choose_image_extent(request4, min_extent, max_extent, current_extent);
    EXPECT_EQ(result.width, max_extent.width);
    EXPECT_EQ(result.height, max_extent.height);
}

TEST(Swapchain, choose_present_mode) {
    const std::vector<VkPresentModeKHR> available_present_modes{
        VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR};

    EXPECT_EQ(Swapchain::choose_present_mode(available_present_modes, {VK_PRESENT_MODE_IMMEDIATE_KHR}),
              VK_PRESENT_MODE_IMMEDIATE_KHR);
    EXPECT_EQ(Swapchain::choose_present_mode(available_present_modes, {VK_PRESENT_MODE_MAILBOX_KHR}),
              VK_PRESENT_MODE_MAILBOX_KHR);
    EXPECT_EQ(Swapchain::choose_present_mode(available_present_modes, {VK_PRESENT_MODE_FIFO_RELAXED_KHR}),
              VK_PRESENT_MODE_FIFO_RELAXED_KHR);

    // This one is guaranteed to be available in any case
    EXPECT_EQ(Swapchain::choose_present_mode(available_present_modes, {VK_PRESENT_MODE_FIFO_KHR}),
              VK_PRESENT_MODE_FIFO_KHR);
}

TEST(Swapchain, choose_surface_format) {
    const std::vector<VkSurfaceFormatKHR> priority_list1{{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                         {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
    const std::vector<VkSurfaceFormatKHR> priority_list2{
        {VK_FORMAT_R4G4_UNORM_PACK8, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT},
        {VK_FORMAT_R8_SNORM, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT}};
    const auto available_surface_formats = priority_list1;

    const VkSurfaceFormatKHR format1{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    auto result = Swapchain::choose_surface_format(available_surface_formats, priority_list1).value();
    EXPECT_EQ(result.format, format1.format);
    EXPECT_EQ(result.colorSpace, format1.colorSpace);

    EXPECT_EQ(Swapchain::choose_surface_format(priority_list2, priority_list1), std::nullopt);
}

} // namespace inexor::vulkan_renderer::wrapper
