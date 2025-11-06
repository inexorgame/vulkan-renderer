#include "inexor/vulkan-renderer/wrapper/swapchains/swapchain_utils.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/representation.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <ranges>

namespace inexor::vulkan_renderer::wrapper::swapchain {

// Using declaration
using tools::InexorException;

std::uint32_t choose_array_layers(const VkSurfaceCapabilitiesKHR &caps, const std::uint32_t requested_layer_count) {
    return std::clamp(requested_layer_count, 1u, caps.maxImageArrayLayers);
}

VkCompositeAlphaFlagBitsKHR choose_composite_alpha(const VkCompositeAlphaFlagsKHR supported_composite_alpha,
                                                   const VkCompositeAlphaFlagsKHR request_composite_alpha) {
    // Return the requested composite alpha if it's supported.
    if ((request_composite_alpha & supported_composite_alpha) != 0u) {
        const auto result = static_cast<VkCompositeAlphaFlagBitsKHR>(request_composite_alpha);
        spdlog::trace("Selecting swapchain composite alpha '{}'", tools::as_string(result));
        return result;
    }
    // If the requested composite alpha is not supported, pick one of these as fallback if available.
    static constexpr std::array<VkCompositeAlphaFlagBitsKHR, 4> composite_alpha_flags{
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    // Find the first supported flag using ranges.
    auto it = std::ranges::find_if(composite_alpha_flags, [&](const auto flag) {
        // Check if the fallback composite alpha is supported.
        return (flag & supported_composite_alpha) != 0u;
    });
    // Check if an alternative composite alpha flag could be found as fallback.
    if (it != composite_alpha_flags.end()) {
        spdlog::trace("Swapchain composite alpha '{}' is not supported, selecting '{}'",
                      tools::as_string(static_cast<VkCompositeAlphaFlagBitsKHR>(request_composite_alpha)),
                      tools::as_string(*it));
        return *it;
    }
    // Throw an exception if the requested composite alpha is not supported and no fallback was found.
    throw InexorException("Error: No compatible swapchain composite alpha found!");
}

std::uint32_t choose_image_count(const VkSurfaceCapabilitiesKHR &caps, const std::uint32_t frames_in_flight) {
    auto img_count = caps.minImageCount + frames_in_flight;
    // If max image count is specified and our image count exceeds this,
    // clamp the max image count to limits as defined by surface capabilities.
    if (caps.maxImageCount > 0 && img_count > caps.maxImageCount) {
        img_count = caps.maxImageCount;
    }
    spdlog::trace("Selecting swapchain image count {}", img_count);
    return img_count;
}

VkExtent2D choose_image_extent(const VkExtent2D &requested_extent, const VkSurfaceCapabilitiesKHR &caps,
                               const VkExtent2D &current_extent) {
    VkExtent2D result{};
    // If the surface specifies the extent (most common case), just use it.
    if (current_extent.width != std::numeric_limits<std::uint32_t>::max() && current_extent.width != 0 &&
        current_extent.height != 0) {
        result = current_extent;
    } else {
        // Otherwise, choose requested or fallback dimensions, clamped to supported range.
        result = {
            std::clamp(requested_extent.width ? requested_extent.width : caps.minImageExtent.width,
                       caps.minImageExtent.width, caps.maxImageExtent.width),
            std::clamp(requested_extent.height ? requested_extent.height : caps.minImageExtent.height,
                       caps.minImageExtent.height, caps.maxImageExtent.height),
        };
    }
    spdlog::trace("Selecting swapchain image extent {} x {}", result.width, result.height);
    return result;
}

VkImageUsageFlags choose_image_usage(const VkImageUsageFlags supported_flags,
                                     const VkFormatFeatureFlags supported_format_features,
                                     const VkImageUsageFlags requested_flags) {
    // Only consider swapchain-relevant usage bits.
    constexpr VkImageUsageFlagBits swapchain_relevant[] = {
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_IMAGE_USAGE_SAMPLED_BIT,
    };
    // This lambda is used to validat a swapchain image usage flag bit with respect to the supported format features.
    auto is_supported = [&](VkImageUsageFlagBits bit) -> bool {
        switch (bit) {
        case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:
            return (supported_flags & bit) && (supported_format_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
        case VK_IMAGE_USAGE_SAMPLED_BIT:
            return (supported_flags & bit) && (supported_format_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
        case VK_IMAGE_USAGE_TRANSFER_SRC_BIT:
        case VK_IMAGE_USAGE_TRANSFER_DST_BIT:
            return (supported_flags & bit);
        default:
            return false;
        }
    };
    // Validate requested image usage flags.
    std::set<VkImageUsageFlagBits> validated_flag_bits;
    for (const auto &bit : swapchain_relevant) {
        if ((requested_flags & bit) && is_supported(bit)) {
            validated_flag_bits.insert(bit);
        } else if (requested_flags & bit) {
            spdlog::warn("Requested swapchain usage '{}' is not supported!", tools::as_string(bit));
        }
    }
    // Fallback if no valid swapchain image usage flag bits were found.
    if (validated_flag_bits.empty()) {
        if (is_supported(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
            validated_flag_bits.insert(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
            spdlog::trace("Using COLOR_ATTACHMENT_BIT as fallback");
        } else {
            throw std::runtime_error("No supported swapchain image usage found for the selected format/device!");
        }
    }
    // Pack all flag bits into one bitmask and also generate a string which contains the flags for logging.
    std::string img_usage_flags_str;
    VkImageUsageFlags result = 0;
    for (const auto &flag_bit : validated_flag_bits) {
        result |= flag_bit;
        if (img_usage_flags_str.empty()) {
            img_usage_flags_str = tools::as_string(flag_bit);
        } else {
            img_usage_flags_str.append("|");
            img_usage_flags_str.append(tools::as_string(flag_bit));
        }
    }
    if (result == 0) {
        throw InexorException("Error: Could not find any swapchaim image usage flags!");
    }
    spdlog::trace("Selecting swapchain image usage '{}'", img_usage_flags_str);
    return result;
}

VkPresentModeKHR choose_present_mode(const std::span<const VkPresentModeKHR> available_present_modes,
                                     const bool vsync_enabled) {
    if (available_present_modes.empty()) {
        throw InexorException("Error: Parameter 'available_present_modes' is empty!");
    }
    if (!vsync_enabled) {
        // Define the preferred present modes in order of priority.
        // Note that VK_PRESENT_MODE_FIFO_KHR is not in here so that we can return it as a fallback.
        static constexpr std::array<VkPresentModeKHR, 3> present_modes_in_preference_order{
            VK_PRESENT_MODE_IMMEDIATE_KHR,
            VK_PRESENT_MODE_MAILBOX_KHR,
            VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        };
        // Iterate through the preferred present modes and return the first one that is supported.
        for (auto requested_mode : present_modes_in_preference_order) {
            if (std::ranges::find(available_present_modes, requested_mode) != available_present_modes.end()) {
                return requested_mode;
            }
        }
        // If none of the present modes from the priority list are available, fallback to FIFO.
    }
    // FIFO is guaranteed to be supported and enforces vsync to be enabled.
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR choose_surface_format(const std::span<const VkSurfaceFormatKHR> available_formats,
                                         const std::span<const VkSurfaceFormatKHR> custom_format_prioriy_list) {
    if (available_formats.empty()) {
        throw InexorException("Error: Parameter 'available_formats' is empty!");
    }
    // This lambda is used to find a matching surface format from a priority list,
    // either the priority list that was specified by the user or the default priority list as fallback.
    const auto find_matching_format = [&](const auto &prio_list) -> std::optional<VkSurfaceFormatKHR> {
        for (const auto &requested_format : prio_list) {
            const auto it = std::find_if(available_formats.begin(), available_formats.end(), [&](const auto candidate) {
                return (candidate.format == requested_format.format) &&
                       (candidate.colorSpace == requested_format.colorSpace);
            });
            if (it != available_formats.end()) {
                return *it;
            }
        }
        return std::nullopt;
    };
    // The chosen surface format will be determined from a user defined priority list or the default priority list.
    std::optional<VkSurfaceFormatKHR> chosen_format;
    // If the user specified a custom priority list, attempt to use it.
    if (!custom_format_prioriy_list.empty()) {
        const auto candidate = find_matching_format(custom_format_prioriy_list);
        if (candidate) {
            chosen_format = *candidate;
            spdlog::trace("Selecting surface format '{}' with color space '{}' from custom priority list",
                          tools::as_string(chosen_format->format), tools::as_string(chosen_format->colorSpace));
        } else {
            // We think this might be worth a warning, not a trace.
            spdlog::warn("Could not find any surface format from the priority list of formats");
            spdlog::warn("Attempting to select a surface format from the default priority list as fallback");
        }
    } else {
        spdlog::trace("No custom surface format priority list specified");
        spdlog::trace("Attempting to select a surface format from the default priority list");
    }
    if (!chosen_format) {
        // If no priority list is specified, use the default list.
        static constexpr std::array<VkSurfaceFormatKHR, 2> default_format_priority_list{
        // Which value to prefer depends a lot on the operating system!
#ifdef _WIN32
            // Try these formats for Windows.
            VkSurfaceFormatKHR{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
#else
            // Try these formats for Linux.
            VkSurfaceFormatKHR{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
#endif
        };
        // Try to find a matching candidate from the default format priority list.
        const auto candidate = find_matching_format(default_format_priority_list);
        if (candidate) {
            chosen_format = *candidate;
            spdlog::trace("Selecting swapchain surface format '{}' with color space '{}'",
                          tools::as_string(chosen_format->format), tools::as_string(chosen_format->colorSpace));
        } else {
            // We will throw an exception after this as well.
            spdlog::error("Could not find any matching surface format from default format priority list!");
        }
    }
    // Throw an exception if no surface format could be found.
    if (!chosen_format) {
        throw InexorException("Error: Could not find a matching surface format!");
    }
    return *chosen_format;
}

VkSurfaceTransformFlagBitsKHR choose_transform(const VkSurfaceCapabilitiesKHR &caps,
                                               const VkSurfaceTransformFlagsKHR requested_transform) {
    const auto chosen_transform = ((requested_transform & caps.supportedTransforms) != 0u)
                                      ? static_cast<VkSurfaceTransformFlagBitsKHR>(requested_transform)
                                      : caps.currentTransform;
    spdlog::trace("Selecting swapchain image transform '{}'", tools::as_string(chosen_transform));
    return chosen_transform;
}

} // namespace inexor::vulkan_renderer::wrapper::swapchain
