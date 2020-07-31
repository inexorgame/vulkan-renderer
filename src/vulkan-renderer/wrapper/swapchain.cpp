#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/wrapper/info.hpp"
#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"

#include <spdlog/spdlog.h>

#include <limits>
#include <optional>

namespace inexor::vulkan_renderer::wrapper {

Swapchain::Swapchain(Swapchain &&other) noexcept
    : device(other.device), graphics_card(std::exchange(other.graphics_card, nullptr)),
      surface(std::exchange(other.surface, nullptr)), swapchain(std::exchange(other.swapchain, nullptr)),
      surface_format(other.surface_format), extent(other.extent), swapchain_images(std::move(other.swapchain_images)),
      swapchain_image_views(std::move(other.swapchain_image_views)), swapchain_image_count(other.swapchain_image_count),
      vsync_enabled(other.vsync_enabled), name(std::move(other.name)) {}

void Swapchain::setup_swapchain(const VkSwapchainKHR old_swapchain, std::uint32_t window_width,
                                std::uint32_t window_height) {
    VulkanSettingsDecisionMaker settings_decision_maker;

    settings_decision_maker.decide_width_and_height_of_swapchain_extent(graphics_card, surface, window_width,
                                                                        window_height, extent);

    std::optional<VkPresentModeKHR> present_mode =
        settings_decision_maker.decide_which_presentation_mode_to_use(graphics_card, surface, vsync_enabled);

    if (!present_mode) {
        throw std::runtime_error("Error: Could not find a suitable present mode!");
    }

    swapchain_image_count = settings_decision_maker.decide_how_many_images_in_swapchain_to_use(graphics_card, surface);

    auto surface_format_candidate =
        settings_decision_maker.decide_which_surface_color_format_in_swapchain_to_use(graphics_card, surface);

    if (!surface_format_candidate) {
        throw std::runtime_error("Error: Could not find an image format for images in swapchain!");
    }

    surface_format = *surface_format_candidate;

    auto swapchain_ci = make_info<VkSwapchainCreateInfoKHR>();
    swapchain_ci.surface = surface;
    swapchain_ci.minImageCount = swapchain_image_count;
    swapchain_ci.imageFormat = surface_format.format;
    swapchain_ci.imageColorSpace = surface_format.colorSpace;
    swapchain_ci.imageExtent.width = extent.width;
    swapchain_ci.imageExtent.height = extent.height;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.preTransform =
        (VkSurfaceTransformFlagBitsKHR)settings_decision_maker.decide_which_image_transformation_to_use(graphics_card,
                                                                                                        surface);
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.queueFamilyIndexCount = 0;
    swapchain_ci.pQueueFamilyIndices = nullptr;
    swapchain_ci.presentMode = *present_mode;

    if (old_swapchain != VK_NULL_HANDLE) {
        swapchain_ci.oldSwapchain = old_swapchain;
    }

    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area.
    swapchain_ci.clipped = VK_TRUE;
    swapchain_ci.compositeAlpha = settings_decision_maker.find_composite_alpha_format(graphics_card, surface);

    // Set additional usage flag for blitting from the swapchain images if supported.
    VkFormatProperties formatProps;

    vkGetPhysicalDeviceFormatProperties(graphics_card, surface_format.format, &formatProps);

    if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) ||
        (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if (vkCreateSwapchainKHR(device, &swapchain_ci, nullptr, &swapchain) != VK_SUCCESS) {
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

    auto image_view_ci = make_info<VkImageViewCreateInfo>();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = surface_format.format;
    image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount = 1;

    for (std::size_t i = 0; i < swapchain_image_count; i++) {
        spdlog::debug("Creating swapchain image #{}.", i);

        image_view_ci.image = swapchain_images[i];

        if (vkCreateImageView(device, &image_view_ci, nullptr, &swapchain_image_views[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: vkCreateImageView failed!");
        }

        // TODO: Use Vulkan debug markers to assign an appropriate name to this swapchain image view.
    }

    spdlog::debug("Created {} swapchain image views successfully.", swapchain_image_count);
}

Swapchain::Swapchain(const VkDevice device, const VkPhysicalDevice graphics_card, const VkSurfaceKHR surface,
                     std::uint32_t window_width, std::uint32_t window_height, const bool enable_vsync,
                     const std::string &name)
    : device(device), graphics_card(graphics_card), surface(surface), vsync_enabled(enable_vsync), name(name) {

    assert(device);
    assert(graphics_card);
    assert(surface);

    setup_swapchain(VK_NULL_HANDLE, window_width, window_height);
}

std::uint32_t Swapchain::acquire_next_image(const wrapper::Semaphore &semaphore) {
    std::uint32_t image_index;
    vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<std::uint64_t>::max(), semaphore.get(), VK_NULL_HANDLE,
                          &image_index);
    return image_index;
}

void Swapchain::recreate(std::uint32_t window_width, std::uint32_t window_height) {
    // Store the old swapchain. This allows us to pass it to VkSwapchainCreateInfoKHR::oldSwapchain to speed up
    // swapchain recreation.
    VkSwapchainKHR old_swapchain = swapchain;

    // When swapchain needs to be recreated, all the old swapchain images need to be destroyed.

    // Unlike swapchain images, the image views were created by us directly.
    // It is our job to destroy them again.
    for (auto *image_view : swapchain_image_views) {
        vkDestroyImageView(device, image_view, nullptr);
    }

    swapchain_image_views.clear();

    swapchain_images.clear();

    setup_swapchain(old_swapchain, window_width, window_height);
}

Swapchain::~Swapchain() {
    spdlog::trace("Destroying swapchain {}.", name);
    vkDestroySwapchainKHR(device, swapchain, nullptr);

    for (auto *image_view : swapchain_image_views) {
        vkDestroyImageView(device, image_view, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
