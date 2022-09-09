﻿#include "inexor/vulkan-renderer/settings_decision_maker.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
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

std::vector<VkExtensionProperties>
VulkanSettingsDecisionMaker::get_all_device_extension_properties(const VkPhysicalDevice physical_device) {
    std::uint32_t count = 0;
    if (const auto result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }
    std::vector<VkExtensionProperties> extensions(count);
    if (const auto result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }
    return std::move(extensions);
}

bool VulkanSettingsDecisionMaker::is_graphics_card_suitable(const VkPhysicalDevice physical_device,
                                                            const VkSurfaceKHR surface) {
    assert(physical_device);
    assert(surface);

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physical_device, &props);

    const auto extension_props = get_all_device_extension_properties(physical_device);
    if (extension_props.empty()) {
        spdlog::error("No device extensions available for physical device {}!", props.deviceName);
        return false;
    }

    bool swapchain_supported = false;
    for (const auto &prop : extension_props) {
        if (strcmp(prop.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
            swapchain_supported = true;
        }
    }

    if (!swapchain_supported) {
        return false;
    }

    // TODO: Check if the selected device supports queue families for graphics bits and presentation!
    VkBool32 presentation_supported = 0;
    if (const auto result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, 0, surface, &presentation_supported);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceSupportKHR failed!", result);
    }

    return presentation_supported == VK_TRUE;
}

VkPhysicalDeviceType VulkanSettingsDecisionMaker::graphics_card_type(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(graphics_card, &props);
    return props.deviceType;
}

std::int32_t VulkanSettingsDecisionMaker::rate_physical_device(const VkPhysicalDevice graphics_card,
                                                               const VkSurfaceKHR surface) {
    assert(graphics_card);
    assert(surface);

    if (!is_graphics_card_suitable(graphics_card, surface)) {
        return -1;
    }

    // We prefer discrete physical devices over integrated ones
    std::int32_t type_score = 1;
    switch (graphics_card_type(graphics_card)) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        type_score = 10;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        type_score = 2;
        break;
    default:
        break;
    }

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(graphics_card, &mem_props);

    // Summarize real GPU memory in megabytes as a factor for the rating
    std::int32_t mem_score = 0;
    for (std::size_t i = 0; i < mem_props.memoryHeapCount; i++) {
        if ((mem_props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0) {
            mem_score += mem_props.memoryHeaps[i].size / (1000 * 1000);
        }
    }
    return type_score * mem_score;
}

std::vector<VkPhysicalDevice> VulkanSettingsDecisionMaker::get_all_physical_devices(const VkInstance inst) {
    // Query how many physical devices are available
    std::uint32_t physical_device_count = 0;
    if (const auto result = vkEnumeratePhysicalDevices(inst, &physical_device_count, nullptr); result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
    }
    // Get information about the available graphics cards
    std::vector<VkPhysicalDevice> phys_devices(physical_device_count);
    if (const auto result = vkEnumeratePhysicalDevices(inst, &physical_device_count, phys_devices.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
    }
    return std::move(phys_devices);
}

std::optional<VkPhysicalDevice>
VulkanSettingsDecisionMaker::pick_graphics_card(const VkInstance inst, const VkSurfaceKHR surface,
                                                const std::optional<std::uint32_t> preferred_index) {
    assert(inst);
    assert(surface);

    auto physical_devices = get_all_physical_devices(inst);
    if (physical_devices.empty()) {
        throw std::runtime_error("Error: No physical devices available!");
    }

    // Did the user specify the index of a prefered physical device?
    if (preferred_index) {
        // Is the index even valid?
        if (*preferred_index < physical_devices.size()) {
            if (rate_physical_device(physical_devices[*preferred_index], surface) > 0) {
                return physical_devices[*preferred_index];
            }
            spdlog::error("The prefered physical device is unsuitable!");
        }
        spdlog::error("The specified index for a prefered physical device is invalid!");
    }

    std::sort(physical_devices.begin(), physical_devices.end(),
              [&](const VkPhysicalDevice lhs, const VkPhysicalDevice rhs) {
                  return rate_physical_device(lhs, surface) < rate_physical_device(rhs, surface);
              });

    return physical_devices.front();
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

std::optional<std::uint32_t>
VulkanSettingsDecisionMaker::find_graphics_queue_family(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::trace("There are {} queue families available", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families,
                                             available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            // Check if this queue family supports graphics.
            if ((available_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                // Ok this queue family supports graphics!
                return static_cast<std::uint32_t>(i);
            }
        }
    }

    // In this case we could not find any suitable graphics queue family!
    return std::nullopt;
}

std::optional<std::uint32_t>
VulkanSettingsDecisionMaker::find_presentation_queue_family(const VkPhysicalDevice graphics_card,
                                                            const VkSurfaceKHR surface) {
    assert(graphics_card);
    assert(surface);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::trace("There are {} queue families available", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families,
                                             available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            const auto this_queue_family_index = static_cast<std::uint32_t>(i);

            VkBool32 presentation_available = 0;

            // Query if presentation is supported.
            if (const auto result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, this_queue_family_index,
                                                                         surface, &presentation_available);
                result != VK_SUCCESS) {
                throw VulkanException("Error: vkGetPhysicalDeviceSurfaceSupportKHR failed!", result);
            }

            if (presentation_available != 0) {
                return this_queue_family_index;
            }
        }
    }

    // In this case we could not find any suitable present queue family!
    return std::nullopt;
}

std::optional<std::uint32_t>
VulkanSettingsDecisionMaker::find_distinct_data_transfer_queue_family(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::trace("There are {} queue families available", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families,
                                             available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            // A distinct transfer queue has a transfer bit set but no graphics bit.
            if ((available_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                if ((available_queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
                    auto this_queue_family_index = static_cast<std::uint32_t>(i);
                    return this_queue_family_index;
                }
            }
        }
    }

    // In this case we could not find any distinct transfer queue family!
    return std::nullopt;
}

std::optional<std::uint32_t>
VulkanSettingsDecisionMaker::find_any_data_transfer_queue_family(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::trace("There are {} queue families available", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families,
                                             available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            // All we care about is VK_QUEUE_TRANSFER_BIT.
            // It is very likely that this queue family has VK_QUEUE_GRAPHICS_BIT as well!
            if ((available_queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
                auto this_queue_family_index = static_cast<std::uint32_t>(i);
                return this_queue_family_index;
            }
        }
    }

    // In this case we could not find any suitable transfer queue family at all!
    // Data transfer from CPU to GPU is not possible in this case!
    return std::nullopt;
}

std::optional<std::uint32_t>
VulkanSettingsDecisionMaker::find_queue_family_for_both_graphics_and_presentation(const VkPhysicalDevice graphics_card,
                                                                                  const VkSurfaceKHR surface) {
    assert(graphics_card);
    assert(surface);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::trace("There are {} queue families available", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families,
                                             available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            // Check if this queue family supports graphics.
            if ((available_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                // Ok this queue family supports graphics!
                // Now let's check if it supports presentation.
                VkBool32 presentation_available = 0;

                auto this_queue_family_index = static_cast<std::uint32_t>(i);

                // Query if presentation is supported.
                if (const auto result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, this_queue_family_index,
                                                                             surface, &presentation_available);
                    result != VK_SUCCESS) {
                    throw VulkanException("Error: vkGetPhysicalDeviceSurfaceSupportKHR failed!", result);
                }

                // Check if we can use this queue family for presentation as well.
                if (presentation_available != 0) {
                    spdlog::trace("Found one queue family for both graphics and presentation");
                    return this_queue_family_index;
                }
            }
        }
    }

    // There is no queue which supports both graphics and presentation.
    // We have to used 2 separate queues then!
    return std::nullopt;
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
