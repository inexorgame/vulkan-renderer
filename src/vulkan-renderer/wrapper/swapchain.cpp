#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include "inexor/vulkan-renderer/settings_decision_maker.hpp"

#include <spdlog/spdlog.h>

#include <optional>

namespace inexor::vulkan_renderer::wrapper {

Swapchain::Swapchain(Swapchain &&other) noexcept
    : device(other.device), graphics_card(std::exchange(other.graphics_card, nullptr)), surface(std::exchange(other.surface, nullptr)),
      swapchain(std::exchange(other.swapchain, nullptr)), surface_format(other.surface_format), extent(other.extent),
      swapchain_images(std::move(other.swapchain_images)), swapchain_image_views(std::move(other.swapchain_image_views)),
      swapchain_image_count(other.swapchain_image_count), vsync_enabled(other.vsync_enabled) {}

void Swapchain::setup_swapchain(const VkSwapchainKHR old_swapchain, std::uint32_t window_width, std::uint32_t window_height) {
    VulkanSettingsDecisionMaker settings_decision_maker;

    settings_decision_maker.decide_width_and_height_of_swapchain_extent(graphics_card, surface, window_width, window_height, extent);

    std::optional<VkPresentModeKHR> present_mode = settings_decision_maker.decide_which_presentation_mode_to_use(graphics_card, surface, vsync_enabled);

    if (!present_mode) {
        throw std::runtime_error("Error: Could not find a suitable present mode!");
    }

    swapchain_image_count = settings_decision_maker.decide_how_many_images_in_swapchain_to_use(graphics_card, surface);

    auto surface_format_candidate = settings_decision_maker.decide_which_surface_color_format_in_swapchain_to_use(graphics_card, surface);

    if (!surface_format_candidate) {
        throw std::runtime_error("Error: Could not find an image format for images in swapchain!");
    }

    surface_format = surface_format_candidate.value();

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = swapchain_image_count;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent.width = extent.width;
    swapchain_create_info.imageExtent.height = extent.height;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform =
        (VkSurfaceTransformFlagBitsKHR)settings_decision_maker.decide_which_image_transformation_to_use(graphics_card, surface);
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = nullptr;
    swapchain_create_info.presentMode = present_mode.value();

    if (old_swapchain != VK_NULL_HANDLE) {
        swapchain_create_info.oldSwapchain = old_swapchain;
    }

    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area.
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.compositeAlpha = settings_decision_maker.find_composite_alpha_format(graphics_card, surface);

    // Set additional usage flag for blitting from the swapchain images if supported.
    VkFormatProperties formatProps;

    vkGetPhysicalDeviceFormatProperties(graphics_card, surface_format.format, &formatProps);

    if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) || (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if (vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateSwapchainKHR failed!");
    }

    if (vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkGetSwapchainImagesKHR failed!");
    }

    swapchain_images.resize(swapchain_image_count);

    if (vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images.data())) {
        throw std::runtime_error("Error: vkGetSwapchainImagesKHR failed!");
    }

    // TODO: Assign an appropriate debug marker name to the swapchain images.

    spdlog::debug("Creating {} swapchain image views.", swapchain_image_count);

    swapchain_image_views.resize(swapchain_image_count);

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = surface_format.format;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    for (std::size_t i = 0; i < swapchain_image_count; i++) {
        spdlog::debug("Creating swapchain image #{}.", i);

        image_view_create_info.image = swapchain_images[i];

        if (vkCreateImageView(device, &image_view_create_info, nullptr, &swapchain_image_views[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: vkCreateImageView failed!");
        }

        // TODO: Use Vulkan debug markers to assign an appropriate name to this swapchain image view.
    }

    spdlog::debug("Created {} swapchain image views successfully.", swapchain_image_count);
}

Swapchain::Swapchain(const VkDevice device, const VkPhysicalDevice graphics_card, const VkSurfaceKHR surface, std::uint32_t window_width,
                     std::uint32_t window_height, const bool enable_vsync)
    : device(device), graphics_card(graphics_card), surface(surface), vsync_enabled(enable_vsync) {

    assert(device);
    assert(graphics_card);
    assert(surface);

    setup_swapchain(VK_NULL_HANDLE, window_width, window_height);
}

void Swapchain::recreate(std::uint32_t window_width, std::uint32_t window_height) {
    // Store the old swapchain. This allows us to pass it to VkSwapchainCreateInfoKHR::oldSwapchain to speed up swapchain recreation.
    VkSwapchainKHR old_swapchain = swapchain;

    // When swapchain needs to be recreated, all the old swapchain images need to be destroyed.

    // Unlike swapchain images, the image views were created by us directly.
    // It is our job to destroy them again.
    for (const auto image_view : swapchain_image_views) {
        vkDestroyImageView(device, image_view, nullptr);
    }

    swapchain_image_views.clear();

    swapchain_images.clear();

    setup_swapchain(old_swapchain, window_width, window_height);
}

Swapchain::~Swapchain() {
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    swapchain = VK_NULL_HANDLE;

    for (const auto image_view : swapchain_image_views) {
        vkDestroyImageView(device, image_view, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
