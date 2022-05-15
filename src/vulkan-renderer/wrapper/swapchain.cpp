#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"

#include <spdlog/spdlog.h>

#include <limits>
#include <optional>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Swapchain::Swapchain(const Device &device, const VkSurfaceKHR surface, std::uint32_t window_width,
                     std::uint32_t window_height, const bool enable_vsync, std::string name)
    : m_device(device), m_surface(surface), m_vsync_enabled(enable_vsync), m_name(std::move(name)) {
    assert(device.device());
    assert(surface);

    setup_swapchain(VK_NULL_HANDLE, window_width, window_height);
}

Swapchain::Swapchain(Swapchain &&other) noexcept : m_device(other.m_device) {
    m_surface = std::exchange(other.m_surface, nullptr);
    m_swapchain = std::exchange(other.m_swapchain, nullptr);
    m_surface_format = other.m_surface_format;
    m_extent = other.m_extent;
    m_swapchain_images = std::move(other.m_swapchain_images);
    m_swapchain_image_views = std::move(other.m_swapchain_image_views);
    m_swapchain_image_count = other.m_swapchain_image_count;
    m_vsync_enabled = other.m_vsync_enabled;
    m_name = std::move(other.m_name);
}

void Swapchain::setup_swapchain(const VkSwapchainKHR old_swapchain, std::uint32_t window_width,
                                std::uint32_t window_height) {
    auto swapchain_settings = VulkanSettingsDecisionMaker::swapchain_extent(m_device.physical_device(), m_surface,
                                                                            window_width, window_height);

    m_extent = swapchain_settings.swapchain_size;

    std::optional<VkPresentModeKHR> present_mode =
        VulkanSettingsDecisionMaker::decide_present_mode(m_device.physical_device(), m_surface, m_vsync_enabled);

    if (!present_mode) {
        throw std::runtime_error("Error: Could not find a suitable present mode!");
    }

    m_swapchain_image_count = VulkanSettingsDecisionMaker::swapchain_image_count(m_device.physical_device(), m_surface);

    auto surface_format_candidate =
        VulkanSettingsDecisionMaker::swapchain_surface_color_format(m_device.physical_device(), m_surface);

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
    swapchain_ci.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(
        VulkanSettingsDecisionMaker::image_transform(m_device.physical_device(), m_surface));
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.queueFamilyIndexCount = 0;
    swapchain_ci.pQueueFamilyIndices = nullptr;
    swapchain_ci.presentMode = *present_mode;

    // Swapchain recreation can be accelerated a lot when storing the old swapchain.
    if (old_swapchain != VK_NULL_HANDLE) {
        swapchain_ci.oldSwapchain = old_swapchain;
    }

    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area.
    swapchain_ci.clipped = VK_TRUE;

    const auto &composite_alpha =
        VulkanSettingsDecisionMaker::find_composite_alpha_format(m_device.physical_device(), m_surface);

    if (!composite_alpha) {
        throw std::runtime_error("Error: Could not find composite alpha format while recreating swapchain!");
    }

    swapchain_ci.compositeAlpha = composite_alpha.value();

    // Set additional usage flag for blitting from the swapchain images if supported.
    VkFormatProperties format_properties;

    vkGetPhysicalDeviceFormatProperties(m_device.physical_device(), m_surface_format.format, &format_properties);

    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) != 0 ||
        (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) != 0) {
        swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    m_device.create_swapchain(swapchain_ci, &m_swapchain, m_name);

    if (const auto result = vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &m_swapchain_image_count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetSwapchainImagesKHR failed!", result);
    }

    m_swapchain_images.resize(m_swapchain_image_count);

    if (const auto result = vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &m_swapchain_image_count,
                                                    m_swapchain_images.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetSwapchainImagesKHR failed!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_swapchain, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, m_name);

    spdlog::trace("Creating {} swapchain image views", m_swapchain_image_count);

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
        image_view_ci.image = m_swapchain_images[i];
        m_device.create_image_view(image_view_ci, &m_swapchain_image_views[i], m_name);
    }
}

std::uint32_t Swapchain::acquire_next_image(const Semaphore &semaphore) {
    std::uint32_t image_index = 0;
    vkAcquireNextImageKHR(m_device.device(), m_swapchain, std::numeric_limits<std::uint64_t>::max(), semaphore.get(),
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
        vkDestroyImageView(m_device.device(), image_view, nullptr);
    }

    m_swapchain_image_views.clear();

    m_swapchain_images.clear();

    setup_swapchain(old_swapchain, window_width, window_height);
}

Swapchain::~Swapchain() {
    vkDestroySwapchainKHR(m_device.device(), m_swapchain, nullptr);

    for (auto *image_view : m_swapchain_image_views) {
        vkDestroyImageView(m_device.device(), image_view, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
