#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/wrapper/info.hpp"
#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"

#include <spdlog/spdlog.h>

#include <limits>
#include <optional>

namespace inexor::vulkan_renderer::wrapper {

Swapchain::Swapchain(const VkDevice device, const VkPhysicalDevice graphics_card, const VkSurfaceKHR surface,
                     std::uint32_t window_width, std::uint32_t window_height, const bool enable_vsync,
                     const std::string &name)
    : m_device(device), m_graphics_card(graphics_card), m_surface(surface), m_vsync_enabled(enable_vsync),
      m_name(name) {

    assert(device);
    assert(graphics_card);
    assert(surface);

    setup_swapchain(VK_NULL_HANDLE, window_width, window_height);
}

Swapchain::Swapchain(Swapchain &&other) noexcept
    : m_device(other.m_device), m_graphics_card(std::exchange(other.m_graphics_card, nullptr)),
      m_surface(std::exchange(other.m_surface, nullptr)), m_swapchain(std::exchange(other.m_swapchain, nullptr)),
      m_surface_format(other.m_surface_format), m_extent(other.m_extent),
      m_swapchain_images(std::move(other.m_swapchain_images)),
      m_swapchain_image_views(std::move(other.m_swapchain_image_views)),
      m_swapchain_image_count(other.m_swapchain_image_count), m_vsync_enabled(other.m_vsync_enabled),
      m_name(std::move(other.m_name)) {}

void Swapchain::setup_swapchain(const VkSwapchainKHR old_swapchain, std::uint32_t window_width,
                                std::uint32_t window_height) {
    VulkanSettingsDecisionMaker settings_decision_maker;

    settings_decision_maker.decide_width_and_height_of_swapchain_extent(m_graphics_card, m_surface, window_width,
                                                                        window_height, m_extent);

    std::optional<VkPresentModeKHR> present_mode =
        settings_decision_maker.decide_which_presentation_mode_to_use(m_graphics_card, m_surface, m_vsync_enabled);

    if (!present_mode) {
        throw std::runtime_error("Error: Could not find a suitable present mode!");
    }

    m_swapchain_image_count =
        settings_decision_maker.decide_how_many_images_in_swapchain_to_use(m_graphics_card, m_surface);

    auto surface_format_candidate =
        settings_decision_maker.decide_which_surface_color_format_in_swapchain_to_use(m_graphics_card, m_surface);

    if (!surface_format_candidate) {
        throw std::runtime_error("Error: Could not find an image format for images in swapchain!");
    }

    m_surface_format = *surface_format_candidate;

    auto swapchain_ci = make_info<VkSwapchainCreateInfoKHR>();
    swapchain_ci.surface = m_surface;
    swapchain_ci.minImageCount = m_swapchain_image_count;
    swapchain_ci.imageFormat = m_surface_format.format;
    swapchain_ci.imageColorSpace = m_surface_format.colorSpace;
    swapchain_ci.imageExtent.width = m_extent.width;
    swapchain_ci.imageExtent.height = m_extent.height;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.preTransform =
        (VkSurfaceTransformFlagBitsKHR)settings_decision_maker.decide_which_image_transformation_to_use(m_graphics_card,
                                                                                                        m_surface);
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
    swapchain_ci.compositeAlpha = settings_decision_maker.find_composite_alpha_format(m_graphics_card, m_surface);

    // Set additional usage flag for blitting from the swapchain images if supported.
    VkFormatProperties formatProps;

    vkGetPhysicalDeviceFormatProperties(m_graphics_card, m_surface_format.format, &formatProps);

    if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) ||
        (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if (vkCreateSwapchainKHR(m_device, &swapchain_ci, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateSwapchainKHR failed!");
    }

    if (vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchain_image_count, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkGetSwapchainImagesKHR failed!");
    }

    m_swapchain_images.resize(m_swapchain_image_count);

    if (vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchain_image_count, m_swapchain_images.data())) {
        throw std::runtime_error("Error: vkGetSwapchainImagesKHR failed!");
    }

    // TODO: Assign an appropriate debug marker name to the swapchain images.

    spdlog::debug("Creating {} swapchain image views.", m_swapchain_image_count);

    m_swapchain_image_views.resize(m_swapchain_image_count);

    auto image_view_ci = make_info<VkImageViewCreateInfo>();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = m_surface_format.format;
    image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount = 1;

    for (std::size_t i = 0; i < m_swapchain_image_count; i++) {
        spdlog::debug("Creating swapchain image #{}.", i);

        image_view_ci.image = m_swapchain_images[i];

        if (vkCreateImageView(m_device, &image_view_ci, nullptr, &m_swapchain_image_views[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: vkCreateImageView failed!");
        }

        // TODO: Use Vulkan debug markers to assign an appropriate name to this swapchain image view.
    }

    spdlog::debug("Created {} swapchain image views successfully.", m_swapchain_image_count);
}

std::uint32_t Swapchain::acquire_next_image(const wrapper::Semaphore &semaphore) {
    std::uint32_t image_index;
    vkAcquireNextImageKHR(m_device, m_swapchain, std::numeric_limits<std::uint64_t>::max(), semaphore.get(),
                          VK_NULL_HANDLE, &image_index);
    return image_index;
}

void Swapchain::recreate(std::uint32_t window_width, std::uint32_t window_height) {
    // Store the old swapchain. This allows us to pass it to VkSwapchainCreateInfoKHR::oldSwapchain to speed up
    // swapchain recreation.
    VkSwapchainKHR old_swapchain = m_swapchain;

    // When swapchain needs to be recreated, all the old swapchain images need to be destroyed.

    // Unlike swapchain images, the image views were created by us directly.
    // It is our job to destroy them again.
    for (auto *image_view : m_swapchain_image_views) {
        vkDestroyImageView(m_device, image_view, nullptr);
    }

    m_swapchain_image_views.clear();

    m_swapchain_images.clear();

    setup_swapchain(old_swapchain, window_width, window_height);
}

Swapchain::~Swapchain() {
    spdlog::trace("Destroying swapchain {}.", m_name);
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    for (auto *image_view : m_swapchain_image_views) {
        vkDestroyImageView(m_device, image_view, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
