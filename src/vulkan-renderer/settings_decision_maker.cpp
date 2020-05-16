#include "inexor/vulkan-renderer/settings_decision_maker.hpp"

namespace inexor::vulkan_renderer {

std::uint32_t VulkanSettingsDecisionMaker::decide_how_many_images_in_swapchain_to_use(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface) {
    assert(graphics_card);
    assert(surface);

    spdlog::debug("Deciding automatically how many images in swapchain to use.");

    std::uint32_t number_of_images_in_swapchain = 0;

    VkSurfaceCapabilitiesKHR surface_capabilities = {};

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
    vulkan_error_check(result);

    // TODO: Refactor! How many images do we actually need? Is triple buffering the best option?

    // Determine how many images in swapchain to use.
    number_of_images_in_swapchain = surface_capabilities.minImageCount + 1;

    // If the maximum number of images available in swapchain is greater than our current number, chose it.
    if ((surface_capabilities.maxImageCount > 0) && (surface_capabilities.maxImageCount < number_of_images_in_swapchain)) {
        number_of_images_in_swapchain = surface_capabilities.maxImageCount;
    }

    return number_of_images_in_swapchain;
}

std::optional<VkSurfaceFormatKHR> VulkanSettingsDecisionMaker::decide_which_surface_color_format_in_swapchain_to_use(const VkPhysicalDevice &graphics_card,
                                                                                                                     const VkSurfaceKHR &surface) {
    assert(graphics_card);
    assert(surface);

    spdlog::debug("Deciding automatically which surface color format in swapchain to use.");

    std::uint32_t number_of_available_surface_formats = 0;

    // First check how many surface formats are available.
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, surface, &number_of_available_surface_formats, nullptr);
    if (result != VK_SUCCESS) {
        std::string error_message = get_error_description_text(result);
        display_error_message(error_message);
        return std::nullopt;
    }

    if (0 == number_of_available_surface_formats) {
        std::string error_message = "Error: No surface formats could be found by fpGetPhysicalDeviceSurfaceFormatsKHR!";
        display_error_message(error_message);
        return std::nullopt;
    }

    // Preallocate memory for available surface formats.
    std::vector<VkSurfaceFormatKHR> available_surface_formats(number_of_available_surface_formats);

    // Get information about all surface formats available.
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, surface, &number_of_available_surface_formats, available_surface_formats.data());
    vulkan_error_check(result);

    VkSurfaceFormatKHR accepted_color_format = {};

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM.
    if (number_of_available_surface_formats == 1 && available_surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        accepted_color_format.format = VK_FORMAT_B8G8R8A8_UNORM;
        accepted_color_format.colorSpace = available_surface_formats[0].colorSpace;
    } else {
        // This vector contais all the formats that we can accept.
        // Currently we use VK_FORMAT_B8G8R8A8_UNORM only, since it's the norm.
        std::vector<VkFormat> accepted_formats = {
            VK_FORMAT_B8G8R8A8_UNORM
            // Add more accepted formats here..
        };

        bool found_acceptable_format = false;

        // Loop through the list of available surface formats and compare with the list of acceptable formats.
        for (auto &&surface_format : available_surface_formats) {
            for (auto &accepted_format : accepted_formats) {
                if (surface_format.format == accepted_format) {
                    accepted_color_format.format = surface_format.format;
                    accepted_color_format.colorSpace = surface_format.colorSpace;

                    found_acceptable_format = true;

                    return accepted_color_format;
                }
            }
        }

        // In case VK_FORMAT_B8G8R8A8_UNORM is not available select the first available color format.
        if (!found_acceptable_format) {
            if (!available_surface_formats.empty()) {
                accepted_color_format.format = available_surface_formats[0].format;
                accepted_color_format.colorSpace = available_surface_formats[0].colorSpace;

                return accepted_color_format;
            }
            return std::nullopt;
        }
    }

    return accepted_color_format;
}

bool VulkanSettingsDecisionMaker::is_graphics_card_suitable(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface) {
    assert(graphics_card);
    assert(surface);

    // The properties of the graphics card.
    VkPhysicalDeviceProperties graphics_card_properties;

    // The features of the graphics card.
    VkPhysicalDeviceFeatures graphics_card_features;

    // Get the information about that graphics card's properties.
    vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);

    // Get the information about the graphics card's features.
    vkGetPhysicalDeviceFeatures(graphics_card, &graphics_card_features);

    spdlog::debug("Checking suitability of graphics card: {}.", graphics_card_properties.deviceName);

    // Step 1: Check if swapchain is supported.
    // In theory we could have used the code from VulkanAvailabilityChecks, but I didn't want
    // VulkanSettingsDecisionMaker to have a dependency just because of this one code part here.

    bool swapchain_is_supported = false;

    std::uint32_t number_of_available_device_extensions = 0;

    VkResult result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_available_device_extensions, nullptr);
    vulkan_error_check(result);

    if (number_of_available_device_extensions == 0) {
        display_error_message("Error: No Vulkan device extensions available!");

        // Since there are no device extensions available at all, the desired one is not supported either.
        swapchain_is_supported = false;
    } else {
        // Preallocate memory for device extensions.
        std::vector<VkExtensionProperties> device_extensions(number_of_available_device_extensions);
        result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_available_device_extensions, device_extensions.data());
        vulkan_error_check(result);

        // Loop through all available device extensions and search for the requested one.
        for (const VkExtensionProperties &device_extension : device_extensions) {
            if (strcmp(device_extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                swapchain_is_supported = true;
            }
        }
    }

    if (!swapchain_is_supported) {
        spdlog::debug("This device is not suitable because it does not support swap chain!.");
        return false;
    }

    // Step 2: Check if presentation is supported
    // TODO: Check if the selected device supports queue families for graphics bits and presentation!

    VkBool32 presentation_available = 0;

    // Query if presentation is supported.
    result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, 0, surface, &presentation_available);
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return false;
    }

    if (presentation_available == 0) {
        spdlog::debug("This device is not suitable because it does not support presentation!");
        return false;
    }

    // Add more suitability checks here if neccesary.
    // ....

    return true;
}

VkPhysicalDeviceType VulkanSettingsDecisionMaker::get_graphics_card_type(const VkPhysicalDevice &graphics_card) {
    assert(graphics_card);

    // The properties of the graphics card.
    VkPhysicalDeviceProperties graphics_card_properties;

    // Get the information about that graphics card.
    vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);

    return graphics_card_properties.deviceType;
}

std::size_t VulkanSettingsDecisionMaker::rate_graphics_card(const VkPhysicalDevice &graphics_card) {
    assert(graphics_card);

    // The score of the graphics card.
    std::size_t graphics_card_score = 0;

    // For now, we will only score the memory of the graphics cards.
    // Therefore we calculate the sum of all memory that is

    // Check memory properties of this graphics card.
    VkPhysicalDeviceMemoryProperties graphics_card_memory_properties;

    vkGetPhysicalDeviceMemoryProperties(graphics_card, &graphics_card_memory_properties);

    // Loop through all memory heaps.
    for (std::size_t i = 0; i < graphics_card_memory_properties.memoryHeapCount; i++) {
        auto &propertyFlag = graphics_card_memory_properties.memoryHeaps[i].flags;

        if (propertyFlag & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            // Use real GPU memory as score.
            graphics_card_score += graphics_card_memory_properties.memoryHeaps[i].size / (1000 * 1000);
        }
    }

    // TODO: Check for more features or limits.

    return graphics_card_score;
}

std::optional<VkPhysicalDevice>
VulkanSettingsDecisionMaker::decide_which_graphics_card_to_use(const VkInstance &vulkan_instance, const VkSurfaceKHR &surface,
                                                               const std::optional<std::uint32_t> &preferred_graphics_card_index) {
    assert(vulkan_instance);
    assert(surface);

    // Do not assert preferred_graphics_card_index because this classifies as runtime error!

    std::uint32_t number_of_available_graphics_cards = 0;

    // First check how many graphics cards are available.
    VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_available_graphics_cards, nullptr);
    vulkan_error_check(result);

    if (0 == number_of_available_graphics_cards) {
        // In this case there are not Vulkan compatible graphics cards available!
        display_error_message("Error: Could not find any graphics cards!");
        return std::nullopt;
    }

    // Preallocate memory for the available graphics cards.
    std::vector<VkPhysicalDevice> available_graphics_cards(number_of_available_graphics_cards);

    // Get information about the available graphics cards.
    result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_available_graphics_cards, available_graphics_cards.data());
    vulkan_error_check(result);

    /// ATTEMPT 1
    /// If there is only 1 graphics card available, we don't have a choice and must use that one.
    /// The preferred graphics card index which could have been specified by the user must be either this one or an invalid index!
    if (1 == number_of_available_graphics_cards) {
        spdlog::debug("Because there is only 1 graphics card available, we don't have a choice and must use that one.");

        // Did the user specify a preferred GPU by command line argument?
        // If so, let's take a look at what he wanted us to use.
        // This does not matter in any way in this case.
        if (preferred_graphics_card_index.has_value()) {
            // Since we only have one graphics card to choose from, index 0 is our only option.
            if (0 != preferred_graphics_card_index.value()) {
                spdlog::debug("Ignoring command line argument -gpu {} because there is only one GPU to chose from.", preferred_graphics_card_index.value());
            }
            if (!(preferred_graphics_card_index.value() >= 0 && preferred_graphics_card_index.value() < available_graphics_cards.size())) {
                spdlog::warn("Warning: Array index for selected graphics card would have been invalid anyways!");
            }
        }

        if (is_graphics_card_suitable(available_graphics_cards[0], surface)) {
            spdlog::debug("The only graphics card available is suitable for the application!");
            spdlog::debug("Score: {}", rate_graphics_card(available_graphics_cards[0]));
            return available_graphics_cards[0];
        }
        std::string error_message = "Error: The only graphics card available is unsuitable for the application's purposes!";
        display_error_message(error_message);
        return std::nullopt;
    }

    /// ATTEMPT 2
    /// There is more than 1 graphics cards available, but the user specified which one should be preferred.
    /// It is important to note that the preferred graphics card can be unsuitable for the application's purposes though!
    /// If that is the case, the automatic graphics card selection mechanism is responsible for finding a suitable graphics card.
    /// The user can also simply change the command line argument and try to prefer another graphics card.
    if (preferred_graphics_card_index.has_value()) {
        // Check if this array index is valid!
        if (preferred_graphics_card_index.value() >= 0 && preferred_graphics_card_index < number_of_available_graphics_cards) {
            spdlog::debug("Command line parameter for prefered GPU specified. Checking graphics card with index {}.", preferred_graphics_card_index.value());

            // Check if the graphics card selected by the user meets all the criteria we need!
            if (VulkanSettingsDecisionMaker::is_graphics_card_suitable(available_graphics_cards[preferred_graphics_card_index.value()], surface)) {
                // We are done: Use the graphics card which was specified by the user's command line argument.
                spdlog::debug("The prefered graphics card is suitable for this application.");
                spdlog::debug("Score: {}", rate_graphics_card(available_graphics_cards[preferred_graphics_card_index.value()]));
                return available_graphics_cards[preferred_graphics_card_index.value()];
            }
            spdlog::error("The preferred graphics card with index {} is not suitable for this application!", preferred_graphics_card_index.value());
            spdlog::error("The array index is valid, but this graphics card does not fulfill all requirements!");

            // We are NOT done!
            // Try to select the best graphics card automatically!

        } else {
            // No, this array index for available_graphics_cards is invalid!
            spdlog::error("Error: Invalid command line argument! Graphics card array index {} is invalid!", preferred_graphics_card_index.value());

            // We are NOT done!
            // Try to select the best graphics card automatically!
        }
    } else {
        // Give the user a little hint message.
        spdlog::debug("Info: No command line argument for preferred graphics card given.");
        spdlog::debug("You have more than 1 graphics card available on your machine.");
        spdlog::debug("Specify which one to use by passing -gpu <number> as command line argument.");
        spdlog::debug("Please be aware that the first index is 0.");
    }

    /// ATTEMPT 3
    // There are more than 1 graphics card available and the user did not specify which one to use.
    // If there are exactly 2 graphics card and one of them is VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU and the other one
    // is VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, we should prefere the real graphics card over the integrated one.
    // We also need to check if the VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU one is suitable though!
    // If that is not the case, we check if the VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU is suitable and use it instead.
    // If both are unsuitable, there are no suitable graphics cards available on this machine!
    if (2 == number_of_available_graphics_cards) {
        bool integrated_graphics_card_exists = false;
        bool discrete_graphics_card_exists = false;

        // Both indices are available because number_of_available_graphics_cards is 2.
        VkPhysicalDeviceType gpu_type_1 = get_graphics_card_type(available_graphics_cards[0]);
        VkPhysicalDeviceType gpu_type_2 = get_graphics_card_type(available_graphics_cards[1]);

        if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == gpu_type_1 || VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == gpu_type_2) {
            discrete_graphics_card_exists = true;
        }

        if (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU == gpu_type_1 || VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU == gpu_type_2) {
            integrated_graphics_card_exists = true;
        }

        if (discrete_graphics_card_exists && integrated_graphics_card_exists) {
            // Try to prefer the discrete graphics card over the integrated one!
            VkPhysicalDevice discrete_gpu;
            VkPhysicalDevice integrated_GPU = nullptr;

            if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == gpu_type_1) {
                discrete_gpu = available_graphics_cards[0];
                integrated_GPU = available_graphics_cards[1];
            } else if (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == gpu_type_2) {
                // The other way around.
                discrete_gpu = available_graphics_cards[1];
                integrated_GPU = available_graphics_cards[0];
            }

            // Usually integrated GPUs which do not support Vulkan are not visible to Vulkan's
            // graphics card enumeration. It could be the case that an integrated GPU supports
            // Vulkan but is unsuitable for the application's purposes, since this decision is up to us.

            // Ok, so try to prefer the discrete GPU over the integrated GPU.
            if (is_graphics_card_suitable(discrete_gpu, surface)) {
                spdlog::debug("You have 2 GPUs. The discrete GPU (real graphics card) is suitable for the application. The integrated GPU is not!");
                spdlog::debug("Score: {}", rate_graphics_card(discrete_gpu));

                return discrete_gpu;
            }
            // Ok, so the discrete GPU is unsuitable. What about the integrated GPU?
            if (is_graphics_card_suitable(integrated_GPU, surface)) {
                spdlog::debug("You have 2 GPUs. Surprisingly, the integrated one is suitable for the application. The discrete GPU is not!");
                spdlog::debug("Score: {}", rate_graphics_card(integrated_GPU));

                // This might be a very rare case though.
                return integrated_GPU;
            }
            spdlog::critical("Neither the integrated GPU nor the discrete GPU are suitable!");

            // Neither the integrated GPU nor the discrete GPU are suitable!
            return std::nullopt;
        } else {
            spdlog::debug("Only discrete GPUs available, no integrated graphics.");
        }
    }

    /// ATTEMPT 4
    /// - There are more than 2 graphics cards available.
    /// - Some of them might be suitable for the application.
    /// - The user did no specify a command line argument to prefer a certain graphics card.
    /// - It's not like there are 2 GPUs, one of them a real graphics card and another one an integrated one.
    /// We now have to sort out all the GPU which are unsuitable for the application's purposes.
    /// After this we have to rank them by a score!

    // The suitable graphics cards (by array index).
    std::vector<std::size_t> suitable_graphics_cards;

    // Loop through all available graphics cards and sort out the unsuitable ones.
    for (std::size_t i = 0; i < number_of_available_graphics_cards; i++) {
        if (VulkanSettingsDecisionMaker::is_graphics_card_suitable(available_graphics_cards[i], surface)) {
            spdlog::debug("Adding graphics card index {} to the list of suitable graphics cards", i);

            // Add this graphics card to the list of suitable graphics cards.
            suitable_graphics_cards.push_back(i);
        } else {
            spdlog::debug("Sorting out graphics card index {} because it is unsuitable for this application's purposes!", i);
        }
    }

    // How many graphics cards have been sorted out?
    auto how_many_graphics_card_disqualified = number_of_available_graphics_cards - suitable_graphics_cards.size();

    if (how_many_graphics_card_disqualified > 0) {
        spdlog::debug("{} have been disqualified because they are unsuitable for the application's purposes!", how_many_graphics_card_disqualified);
    }

    // We could not find any suitable graphics card!
    if (suitable_graphics_cards.empty()) {
        spdlog::critical("Error: Could not find suitable graphics card automatically.");
        return std::nullopt;
    }

    // Only 1 graphics card is suitable, let's choose that one.
    if (suitable_graphics_cards.size() == 1) {
        spdlog::debug("There is only 1 suitable graphics card available.");
        spdlog::debug("Score: {}", rate_graphics_card(available_graphics_cards[0]));

        return available_graphics_cards[0];
    }

    // We have more than one suitable graphics card.
    // There is at least one graphics card that is suitable.

    // Use an ordered map to automatically rank graphics cards by score.
    std::multimap<std::size_t, VkPhysicalDevice> graphics_cards_candidates;

    for (const auto &candidate : available_graphics_cards) {
        std::size_t candidate_score = rate_graphics_card(candidate);

        if (candidate_score > 0) {
            // Add it to the candidate map.
            graphics_cards_candidates.insert(std::make_pair(candidate_score, candidate));
        } else {
            // This is extremely unlike but still we have to account for his.
            spdlog::debug("A graphics card has been disqualified because it received a score of 0.");
        }
    }

    // The multimap already ensures that the entries are sorted by key,
    // which is defined as the graphics card's score!

    // Loop through the map using an interator and return the first graphics card which has a score greater than zero.
    // It should be extremy unlikely that a graphics card gets a score of zero after all!
    for (auto candidate_iterator = graphics_cards_candidates.begin(); candidate_iterator != graphics_cards_candidates.end(); candidate_iterator++) {
        spdlog::debug("Score: {}", candidate_iterator->first);

        // We can be sure that the candidate's score is greater than 0 because of the aforementioned code block.
        return candidate_iterator->second;
    }

    // In this case, all available graphics cards are suitable for the application's purposes
    // but scored 0 points in the graphics card score. This is extremely unlikely!
    return std::nullopt;
}

VkSurfaceTransformFlagsKHR VulkanSettingsDecisionMaker::decide_which_image_transformation_to_use(const VkPhysicalDevice &graphics_card,
                                                                                                 const VkSurfaceKHR &surface) {
    assert(graphics_card);
    assert(surface);

    // Bitmask of VkSurfaceTransformFlagBitsKHR.
    VkSurfaceTransformFlagsKHR pre_transform = {};

    VkSurfaceCapabilitiesKHR surface_capabilities = {};

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
    vulkan_error_check(result);

    if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        // We prefer a non-rotated transform.
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        pre_transform = surface_capabilities.currentTransform;
    }

    return pre_transform;
}

// TOOD: std::optional?
VkCompositeAlphaFlagBitsKHR VulkanSettingsDecisionMaker::find_composite_alpha_format(const VkPhysicalDevice selected_graphics_card,
                                                                                     const VkSurfaceKHR surface) {
    VkCompositeAlphaFlagBitsKHR composite_alpha;

    const std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    VkSurfaceCapabilitiesKHR surface_capabilities = {};

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selected_graphics_card, surface, &surface_capabilities);
    vulkan_error_check(result);

    for (auto &composite_alpha_flag : composite_alpha_flags) {
        if (surface_capabilities.supportedCompositeAlpha & composite_alpha_flag) {
            composite_alpha = composite_alpha_flag;
            break;
        };
    }

    return composite_alpha;
}

std::optional<VkPresentModeKHR> VulkanSettingsDecisionMaker::decide_which_presentation_mode_to_use(const VkPhysicalDevice &graphics_card,
                                                                                                   const VkSurfaceKHR &surface, bool vsync) {
    assert(graphics_card);
    assert(surface);

    if (vsync) {
        // VK_PRESENT_MODE_FIFO_KHR specifies that the presentation engine waits for the next vertical blanking period to
        // update the current image. Tearing cannot be observed. An internal queue is used to hold pending presentation requests.
        // New requests are appended to the end of the queue, and one request is removed from the beginning of the queue and processed
        // during each vertical blanking period in which the queue is non-empty. This is the only value of presentMode that is required to be supported.
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    std::uint32_t number_of_available_present_modes = 0;

    // First check how many present modes are available for the selected combination of graphics card and window surface.
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, surface, &number_of_available_present_modes, nullptr);
    vulkan_error_check(result);

    if (0 == number_of_available_present_modes) {
        // According to the spec, this should not even be possible!
        std::string error_message("Error: No presentation modes available!");
        display_error_message(error_message);
        return std::nullopt;
    }

    // Preallocate memory for the available present modes.
    std::vector<VkPresentModeKHR> available_present_modes(number_of_available_present_modes);

    // Get information about the available present modes.
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, surface, &number_of_available_present_modes, available_present_modes.data());
    vulkan_error_check(result);

    for (auto present_mode : available_present_modes) {
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
        // VK_PRESENT_MODE_MAILBOX_KHR specifies that the presentation engine waits for the next vertical blanking period
        // to update the current image. Tearing cannot be observed. An internal single-entry queue is used to hold pending
        // presentation requests. If the queue is full when a new presentation request is received, the new request replaces
        // the existing entry, and any images associated with the prior entry become available for re-use by the application.
        // One request is removed from the queue and processed during each vertical blanking period in which the queue is non-empty.
        if (VK_PRESENT_MODE_MAILBOX_KHR == present_mode) {
            spdlog::debug("VK_PRESENT_MODE_MAILBOX_KHR will be used.");
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    spdlog::warn("VK_PRESENT_MODE_MAILBOX_KHR is not supported by the regarded device.");
    spdlog::warn("Let's see if VK_PRESENT_MODE_IMMEDIATE_KHR is supported.");

    for (auto present_mode : available_present_modes) {
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
        // VK_PRESENT_MODE_IMMEDIATE_KHR specifies that the presentation engine does not wait for a vertical blanking period to update
        // the current image, meaning this mode may result in visible tearing. No internal queuing of presentation requests is needed,
        // as the requests are applied immediately.
        if (VK_PRESENT_MODE_IMMEDIATE_KHR == present_mode) {
            spdlog::debug("VK_PRESENT_MODE_IMMEDIATE_KHR will be used.");
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    spdlog::warn("VK_PRESENT_MODE_IMMEDIATE_KHR is not supported by the regarded device.");

    spdlog::warn("Let's see if VK_PRESENT_MODE_FIFO_KHR is supported.");

    for (auto present_mode : available_present_modes) {
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
        // VK_PRESENT_MODE_FIFO_KHR specifies that the presentation engine waits for the next vertical blanking period to update the current image.
        // Tearing cannot be observed. An internal queue is used to hold pending presentation requests. New requests are appended to the end of the queue,
        // and one request is removed from the beginning of the queue and processed during each vertical blanking period in which the queue is non-empty.
        // This is the only value of presentMode that is required to be supported.
        if (VK_PRESENT_MODE_FIFO_KHR == present_mode) {
            spdlog::debug("VK_PRESENT_MODE_FIFO_KHR will be used.");
            return VK_PRESENT_MODE_FIFO_KHR;
        }
    }

    spdlog::error("VK_PRESENT_MODE_FIFO_KHR is not supported by the regarded device.");
    spdlog::error("Accoring to the Vulkan specification, this shouldn't even be possible!");

    // Lets try with any present mode available!
    if (!available_present_modes.empty()) {
        // Let's just pick the first one.
        return available_present_modes[0];
    }

    // Yes, this might be the case for integrated systems!
    spdlog::critical("The selected graphics card does not support any presentation at all!");

    return std::nullopt;
}

void VulkanSettingsDecisionMaker::decide_width_and_height_of_swapchain_extent(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface,
                                                                              std::uint32_t &window_width, std::uint32_t &window_height,
                                                                              VkExtent2D &swapchain_extent) {
    assert(graphics_card);
    assert(surface);

    VkSurfaceCapabilitiesKHR surface_capabilities = {};

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
    vulkan_error_check(result);

    if (surface_capabilities.currentExtent.width == UINT32_MAX && surface_capabilities.currentExtent.height == UINT32_MAX) {
        // The size of the window dictates the swapchain's extent.
        swapchain_extent.width = window_width;
        swapchain_extent.height = window_height;
    } else {
        // TODO: Refactor! Do it the way the Vulkan Tutorial is doing it.

        // If the surface size is defined, the swap chain size must match.
        swapchain_extent = surface_capabilities.currentExtent;
        window_width = surface_capabilities.currentExtent.width;
        window_height = surface_capabilities.currentExtent.height;
    }
}

std::optional<std::uint32_t> VulkanSettingsDecisionMaker::find_graphics_queue_family(const VkPhysicalDevice &graphics_card) {
    assert(graphics_card);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::debug("There are {} queue families available.", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            // Check if this queue family supports graphics.
            if (available_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                // Ok this queue family supports graphics!
                return static_cast<std::uint32_t>(i);
            }
        }
    }

    // In this case we could not find any suitable graphics queue family!
    return std::nullopt;
}

std::optional<std::uint32_t> VulkanSettingsDecisionMaker::find_presentation_queue_family(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface) {
    assert(graphics_card);
    assert(surface);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::debug("There are {} queue families available.", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            std::uint32_t this_queue_family_index = static_cast<std::uint32_t>(i);

            VkBool32 presentation_available = 0;

            // Query if presentation is supported.
            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, this_queue_family_index, surface, &presentation_available);
            vulkan_error_check(result);

            if (presentation_available != 0) {
                return this_queue_family_index;
            }
        }
    }

    // In this case we could not find any suitable present queue family!
    return std::nullopt;
}

std::optional<std::uint32_t> VulkanSettingsDecisionMaker::find_distinct_data_transfer_queue_family(const VkPhysicalDevice &graphics_card) {
    assert(graphics_card);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::debug("There are {} queue families available.", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            // A distinct transfer queue has a transfer bit set but no graphics bit.
            if (!(available_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                if (available_queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                    auto this_queue_family_index = static_cast<std::uint32_t>(i);
                    return this_queue_family_index;
                }
            }
        }
    }

    // In this case we could not find any distinct transfer queue family!
    return std::nullopt;
}

std::optional<std::uint32_t> VulkanSettingsDecisionMaker::find_any_data_transfer_queue_family(const VkPhysicalDevice &graphics_card) {
    assert(graphics_card);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::debug("There are {} queue families available.", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            // All we care about is VK_QUEUE_TRANSFER_BIT.
            // It is very likely that this queue family has VK_QUEUE_GRAPHICS_BIT as well!
            if (available_queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                auto this_queue_family_index = static_cast<std::uint32_t>(i);
                return this_queue_family_index;
            }
        }
    }

    // In this case we could not find any suitable transfer queue family at all!
    // Data transfer from CPU to GPU is not possible in this case!
    return std::nullopt;
}

std::optional<std::uint32_t> VulkanSettingsDecisionMaker::find_queue_family_for_both_graphics_and_presentation(const VkPhysicalDevice &graphics_card,
                                                                                                               const VkSurfaceKHR &surface) {
    assert(graphics_card);
    assert(surface);

    std::uint32_t number_of_available_queue_families = 0;

    // First check how many queue families are available.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

    spdlog::debug("There are {} queue families available.", number_of_available_queue_families);

    // Preallocate memory for the available queue families.
    std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);

    // Get information about the available queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, available_queue_families.data());

    // Loop through all available queue families and look for a suitable one.
    for (std::size_t i = 0; i < available_queue_families.size(); i++) {
        if (available_queue_families[i].queueCount > 0) {
            // Check if this queue family supports graphics.
            if (available_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                // Ok this queue family supports graphics!
                // Now let's check if it supports presentation.
                VkBool32 presentation_available = 0;

                auto this_queue_family_index = static_cast<std::uint32_t>(i);

                // Query if presentation is supported.
                VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, this_queue_family_index, surface, &presentation_available);
                vulkan_error_check(result);

                // Check if we can use this queue family for presentation as well.
                if (presentation_available != 0) {
                    spdlog::debug("Found one queue family for both graphics and presentation.");
                    return this_queue_family_index;
                }
            }
        }
    }

    // There is no queue which supports both graphics and presentation.
    // We have to used 2 separate queues then!
    return std::nullopt;
}

std::optional<VkFormat> VulkanSettingsDecisionMaker::find_depth_buffer_format(const VkPhysicalDevice &graphics_card, const std::vector<VkFormat> &formats,
                                                                              const VkImageTiling tiling, const VkFormatFeatureFlags feature_flags) {
    spdlog::debug("Trying to find appropriate format for depth buffer.");

    for (const auto &format : formats) {
        VkFormatProperties format_properties;

        vkGetPhysicalDeviceFormatProperties(graphics_card, format, &format_properties);

        if ((tiling == VK_IMAGE_TILING_LINEAR && feature_flags == (format_properties.linearTilingFeatures & feature_flags)) ||
            (tiling == VK_IMAGE_TILING_OPTIMAL && feature_flags == (format_properties.optimalTilingFeatures & feature_flags))) {
            return format;
        }
    }

    return std::nullopt;
}

} // namespace inexor::vulkan_renderer
