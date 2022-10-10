#include "inexor/vulkan-renderer/settings_decision_maker.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer {

std::uint32_t VulkanSettingsDecisionMaker::swapchain_image_count(const VkPhysicalDevice graphics_card,
                                                                 const VkSurfaceKHR surface) {
    assert(graphics_card);
    assert(surface);

    spdlog::trace("Deciding automatically how many images in swapchain to use");

    VkSurfaceCapabilitiesKHR surface_capabilities{};

    if (const auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed!", result);
    }

    // TODO: Refactor! How many images do we actually need? Is triple buffering the best option?

    // Determine how many images in swapchain to use.
    std::uint32_t number_of_images_in_swapchain = surface_capabilities.minImageCount + 1;

    // If the maximum number of images available in swapchain is greater than our current number, chose it.
    if ((surface_capabilities.maxImageCount > 0) &&
        (surface_capabilities.maxImageCount < number_of_images_in_swapchain)) {
        number_of_images_in_swapchain = surface_capabilities.maxImageCount;
    }

    return number_of_images_in_swapchain;
}

std::optional<VkSurfaceFormatKHR>
VulkanSettingsDecisionMaker::swapchain_surface_color_format(const VkPhysicalDevice graphics_card,
                                                            const VkSurfaceKHR surface) {
    assert(graphics_card);
    assert(surface);

    spdlog::trace("Deciding automatically which surface color format in swapchain to use");

    std::uint32_t number_of_available_surface_formats = 0;

    // First check how many surface formats are available.
    if (const auto result =
            vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, surface, &number_of_available_surface_formats, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceFormatsKHR failed!", result);
    }

    if (number_of_available_surface_formats == 0) {
        throw std::runtime_error("Error: No surface formats could be found by fpGetPhysicalDeviceSurfaceFormatsKHR!");
    }

    // Preallocate memory for available surface formats.
    std::vector<VkSurfaceFormatKHR> available_surface_formats(number_of_available_surface_formats);

    // Get information about all surface formats available.
    if (const auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            graphics_card, surface, &number_of_available_surface_formats, available_surface_formats.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceFormatsKHR failed!", result);
    }

    VkSurfaceFormatKHR accepted_color_format{};

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM.
    if (number_of_available_surface_formats == 1 && available_surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        accepted_color_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        accepted_color_format.colorSpace = available_surface_formats[0].colorSpace;
    } else {
        // This vector contains all the formats that we can accept.
        // Currently we use VK_FORMAT_B8G8R8A8_UNORM only, since it's the norm.
        std::vector<VkFormat> accepted_formats = {
            VK_FORMAT_B8G8R8A8_UNORM
            // TODO: Add more accepted formats here..
        };

        // In case VK_FORMAT_B8G8R8A8_UNORM is not available select the first available color format.
        if (!available_surface_formats.empty()) {
            return available_surface_formats[0];
        }

        // Loop through the list of available surface formats and compare with the list of acceptable formats.
        for (auto &surface_format : available_surface_formats) {
            for (auto &accepted_format : accepted_formats) {
                if (surface_format.format == accepted_format) {
                    accepted_color_format.format = surface_format.format;
                    accepted_color_format.colorSpace = surface_format.colorSpace;
                    return surface_format;
                }
            }
        }
    }

    return accepted_color_format;
}

VkSurfaceTransformFlagsKHR VulkanSettingsDecisionMaker::image_transform(const VkPhysicalDevice graphics_card,
                                                                        const VkSurfaceKHR surface) {
    assert(graphics_card);
    assert(surface);

    VkSurfaceCapabilitiesKHR surface_capabilities{};

    if (const auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed!", result);
    }

    if ((surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) == 0) {
        return surface_capabilities.currentTransform;
    }

    return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
}

std::optional<VkCompositeAlphaFlagBitsKHR>
VulkanSettingsDecisionMaker::find_composite_alpha_format(const VkPhysicalDevice selected_graphics_card,
                                                         const VkSurfaceKHR surface) {
    assert(selected_graphics_card);
    assert(surface);

    const std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    VkSurfaceCapabilitiesKHR surface_capabilities{};

    if (const auto result =
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selected_graphics_card, surface, &surface_capabilities);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed!", result);
    }

    for (const auto &composite_alpha_flag : composite_alpha_flags) {
        if ((surface_capabilities.supportedCompositeAlpha & composite_alpha_flag) != 0) {
            return composite_alpha_flag;
        }
    }

    return std::nullopt;
}

std::optional<VkPresentModeKHR> VulkanSettingsDecisionMaker::decide_present_mode(const VkPhysicalDevice graphics_card,
                                                                                 const VkSurfaceKHR surface,
                                                                                 const bool vsync) {
    assert(graphics_card);
    assert(surface);

    if (vsync) {
        // VK_PRESENT_MODE_FIFO_KHR specifies that the presentation engine waits for the next vertical blanking period
        // to update the current image. Tearing cannot be observed. An internal queue is used to hold pending
        // presentation requests. New requests are appended to the end of the queue, and one request is removed from the
        // beginning of the queue and processed during each vertical blanking period in which the queue is non-empty.
        // This is the only value of presentMode that is required to be supported.
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    std::uint32_t number_of_available_present_modes = 0;

    // First check how many present modes are available for the selected combination of graphics card and window
    // surface.
    if (const auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, surface,
                                                                      &number_of_available_present_modes, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfacePresentModesKHR failed!", result);
    }

    if (number_of_available_present_modes == 0) {
        // According to the spec, this should not even be possible!
        spdlog::error("No presentation modes available!");
        return std::nullopt;
    }

    // Preallocate memory for the available present modes.
    std::vector<VkPresentModeKHR> available_present_modes(number_of_available_present_modes);

    // Get information about the available present modes.
    if (const auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            graphics_card, surface, &number_of_available_present_modes, available_present_modes.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfacePresentModesKHR failed!", result);
    }

    for (auto present_mode : available_present_modes) {
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
        // VK_PRESENT_MODE_MAILBOX_KHR specifies that the presentation engine waits for the next vertical blanking
        // period to update the current image. Tearing cannot be observed. An internal single-entry queue is used to
        // hold pending presentation requests. If the queue is full when a new presentation request is received, the new
        // request replaces the existing entry, and any images associated with the prior entry become available for
        // re-use by the application. One request is removed from the queue and processed during each vertical blanking
        // period in which the queue is non-empty.
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            spdlog::trace("VK_PRESENT_MODE_MAILBOX_KHR will be used");
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    spdlog::warn("VK_PRESENT_MODE_MAILBOX_KHR is not supported by the regarded device");
    spdlog::trace("Let's see if VK_PRESENT_MODE_IMMEDIATE_KHR is supported");

    for (auto present_mode : available_present_modes) {
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
        // VK_PRESENT_MODE_IMMEDIATE_KHR specifies that the presentation engine does not wait for a vertical blanking
        // period to update the current image, meaning this mode may result in visible tearing. No internal queuing of
        // presentation requests is needed, as the requests are applied immediately.
        if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            spdlog::trace("VK_PRESENT_MODE_IMMEDIATE_KHR will be used");
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    spdlog::warn("VK_PRESENT_MODE_IMMEDIATE_KHR is not supported by the regarded device");
    spdlog::warn("Let's see if VK_PRESENT_MODE_FIFO_KHR is supported");

    for (auto present_mode : available_present_modes) {
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
        // VK_PRESENT_MODE_FIFO_KHR specifies that the presentation engine waits for the next vertical blanking period
        // to update the current image. Tearing cannot be observed. An internal queue is used to hold pending
        // presentation requests. New requests are appended to the end of the queue, and one request is removed from the
        // beginning of the queue and processed during each vertical blanking period in which the queue is non-empty.
        // This is the only value of presentMode that is required to be supported.
        if (present_mode == VK_PRESENT_MODE_FIFO_KHR) {
            spdlog::trace("VK_PRESENT_MODE_FIFO_KHR will be used");
            return VK_PRESENT_MODE_FIFO_KHR;
        }
    }

    spdlog::error("VK_PRESENT_MODE_FIFO_KHR is not supported by the regarded device");
    spdlog::error("According to the Vulkan specification, this shouldn't even be possible!");

    // Lets try with any present mode available!
    if (!available_present_modes.empty()) {
        // Let's just pick the first one.
        return available_present_modes[0];
    }

    // Yes, this might be the case for integrated systems!
    spdlog::critical("The selected graphics card does not support any presentation at all!");

    return std::nullopt;
}

SwapchainSettings VulkanSettingsDecisionMaker::swapchain_extent(const VkPhysicalDevice graphics_card,
                                                                const VkSurfaceKHR surface,
                                                                const std::uint32_t window_width,
                                                                const std::uint32_t window_height) {
    assert(graphics_card);
    assert(surface);

    VkSurfaceCapabilitiesKHR surface_capabilities{};
    SwapchainSettings updated_swapchain_settings{};

    if (const auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed!", result);
    }

    if (surface_capabilities.currentExtent.width == std::numeric_limits<std::uint32_t>::max() &&
        surface_capabilities.currentExtent.height == std::numeric_limits<std::uint32_t>::max()) {
        // The size of the window dictates the extent of the swapchain.
        updated_swapchain_settings.swapchain_size.width = window_width;
        updated_swapchain_settings.swapchain_size.height = window_height;
    } else {
        // If the surface size is defined, the swap chain size must match.
        updated_swapchain_settings.swapchain_size = surface_capabilities.currentExtent;
        updated_swapchain_settings.window_size = surface_capabilities.currentExtent;
    }

    return updated_swapchain_settings;
}

std::optional<VkFormat>
VulkanSettingsDecisionMaker::find_depth_buffer_format(const VkPhysicalDevice graphics_card,
                                                      const std::vector<VkFormat> &formats, const VkImageTiling tiling,
                                                      const VkFormatFeatureFlags feature_flags) {
    assert(graphics_card);
    assert(!formats.empty());
    assert(tiling);
    assert(feature_flags);

    spdlog::trace("Trying to find appropriate format for depth buffer");

    for (const auto &format : formats) {
        VkFormatProperties format_properties;

        vkGetPhysicalDeviceFormatProperties(graphics_card, format, &format_properties);

        if ((tiling == VK_IMAGE_TILING_LINEAR &&
             feature_flags == (format_properties.linearTilingFeatures & feature_flags)) ||
            (tiling == VK_IMAGE_TILING_OPTIMAL &&
             feature_flags == (format_properties.optimalTilingFeatures & feature_flags))) {
            return format;
        }
    }

    return std::nullopt;
}

} // namespace inexor::vulkan_renderer
