#include "inexor/vulkan-renderer/wrapper/device.hpp"

namespace inexor::vulkan_renderer::wrapper {

Device::Device(Device &&other) noexcept : device(other.device), graphics_card(other.graphics_card) {}

void Device::prepare_queues(bool prefer_distinct_transfer_queue) {
    spdlog::debug("Creating Vulkan device queues.");

    if (prefer_distinct_transfer_queue) {
        spdlog::debug("The application will try to use a distinct data transfer queue if it is available.");
    } else {
        spdlog::warn("The application is forced not to use a distinct data transfer queue!");
    }

    // Check if there is one queue family which can be used for both graphics and presentation.
    std::optional<std::uint32_t> queue_family_index_for_both_graphics_and_presentation =
        settings_decision_maker.find_queue_family_for_both_graphics_and_presentation(graphics_card, surface);

    if (queue_family_index_for_both_graphics_and_presentation.has_value()) {
        spdlog::debug("One queue for both graphics and presentation will be used.");

        graphics_queue_family_index = queue_family_index_for_both_graphics_and_presentation.value();
        present_queue_family_index = graphics_queue_family_index;

        use_one_queue_for_graphics_and_present = true;

        // For now, we only need one queue family.
        std::uint32_t number_of_combined_queues_to_use = 1;

        // In this case, there is one queue family which can be used for both graphics and presentation.
        VkDeviceQueueCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = queue_family_index_for_both_graphics_and_presentation.value();
        create_info.queueCount = number_of_combined_queues_to_use;
        create_info.pQueuePriorities = &global_queue_priority;

        queues_to_create.push_back(create_info);
    } else {
        spdlog::debug("No queue found which supports both graphics and presentation.");
        spdlog::debug("The application will try to use 2 separate queues.");

        // We have to use 2 different queue families.
        // One for graphics and another one for presentation.

        // Check which queue family index can be used for graphics.
        graphics_queue_family_index = settings_decision_maker.find_graphics_queue_family(graphics_card);

        if (!graphics_queue_family_index.has_value()) {
            throw std::runtime_error("Could not find suitable queue family indices for graphics!");
        }

        // Check which queue family index can be used for presentation.
        present_queue_family_index = settings_decision_maker.find_presentation_queue_family(graphics_card, surface);

        if (!present_queue_family_index.has_value()) {
            throw std::runtime_error("Could not find suitable queue family indices for presentation!");
        }

        spdlog::debug("Graphics queue family index: {}.", graphics_queue_family_index.value());
        spdlog::debug("Presentation queue family index: {}.", present_queue_family_index.value());

        // For now, we only need one queue family.
        std::uint32_t number_of_graphics_queues_to_use = 1;

        // Set up one queue for graphics.
        VkDeviceQueueCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = graphics_queue_family_index.value();
        create_info.queueCount = number_of_graphics_queues_to_use;
        create_info.pQueuePriorities = &global_queue_priority;

        queues_to_create.push_back(create_info);

        // For now, we only need one queue family.
        std::uint32_t number_of_present_queues_to_use = 1;

        // Set up one queue for presentation.
        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = present_queue_family_index.value();
        create_info.queueCount = number_of_present_queues_to_use;
        create_info.pQueuePriorities = &global_queue_priority;

        queues_to_create.push_back(create_info);
    }

    // Add another device queue just for data transfer.
    transfer_queue_family_index = settings_decision_maker.find_distinct_data_transfer_queue_family(graphics_card);

    if (transfer_queue_family_index.has_value() && prefer_distinct_transfer_queue) {
        spdlog::debug("A separate queue will be used for data transfer.");
        spdlog::debug("Data transfer queue family index: {}.", transfer_queue_family_index.value());

        // We have the opportunity to use a separated queue for data transfer!
        use_distinct_data_transfer_queue = true;

        // For now, we only need one queue family.
        std::uint32_t number_of_queues_to_use = 1;

        VkDeviceQueueCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.queueFamilyIndex = transfer_queue_family_index.value();
        create_info.queueCount = number_of_queues_to_use;
        create_info.pQueuePriorities = &global_queue_priority;

        queues_to_create.push_back(create_info);
    } else {
        // We don't have the opportunity to use a separated queue for data transfer!
        // Do not create a new queue, use the graphics queue instead.
        use_distinct_data_transfer_queue = false;
    }

    if (!use_distinct_data_transfer_queue) {
        spdlog::warn("The application is forces to avoid distinct data transfer queues.");
        spdlog::warn("Because of this, the graphics queue will be used for data transfer.");

        transfer_queue_family_index = graphics_queue_family_index;
    }
}

Device::Device(bool prefer_distinct_transer_queue, const std::optional<std::uint32_t> preferred_physical_device_index) : graphics_card(graphics_card) {

    prepare_queues(true);

    std::vector<const char *> device_extensions_wishlist = {
        // Since we want to draw on a window, we need the swapchain extension.
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

#ifndef NDEBUG
    // Debug markers allow the assignment of internal names to Vulkan resources.
    // These internal names will conveniently be visible in debuggers like RenderDoc.
    device_extensions_wishlist.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
#endif

    std::vector<const char *> enabled_device_extensions;

    for (auto device_extension_name : device_extensions_wishlist) {
        if (availability_checks.has_device_extension(graphics_card, device_extension_name)) {
            spdlog::debug("Device extension '{}' is available on this system.", device_extension_name);
            enabled_device_extensions.push_back(device_extension_name);
        } else {
            throw std::runtime_error("Device extension " + std::string(device_extension_name) + " is not available on this system!");
        }
    }

    VkPhysicalDeviceFeatures used_features = {};

    // Enable anisotropic filtering.
    used_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = static_cast<std::uint32_t>(queues_to_create.size());
    create_info.pQueueCreateInfos = queues_to_create.data();
    // Device layers were deprecated in Vulkan some time ago, essentially making all layers instance layers.
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
    create_info.enabledExtensionCount = static_cast<std::uint32_t>(enabled_device_extensions.size());
    create_info.ppEnabledExtensionNames = enabled_device_extensions.data();
    create_info.pEnabledFeatures = &used_features;

    spdlog::debug("Creating physical device (graphics card interface).");

    if (vkCreateDevice(graphics_card, &create_info, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateDevice failed!");
    }

    // TODO: Assign internal Vulkan debug marker name to device and physical device.

    spdlog::debug("Initialising GPU queues.");

    spdlog::debug("Graphics queue family index: {}.", graphics_queue_family_index.value());
    spdlog::debug("Presentation queue family index: {}.", present_queue_family_index.value());
    spdlog::debug("Data transfer queue family index: {}.", transfer_queue_family_index.value());

    // Setup the queues for presentation and graphics.
    // Since we only have one queue per queue family, we acquire index 0.
    vkGetDeviceQueue(device, present_queue_family_index.value(), 0, &present_queue);
    vkGetDeviceQueue(device, graphics_queue_family_index.value(), 0, &graphics_queue);

    // The use of data transfer queues can be forbidden by using -no_separate_data_queue.
    if (use_distinct_data_transfer_queue) {
        // Use a separate queue for data transfer to GPU.
        vkGetDeviceQueue(device, transfer_queue_family_index.value(), 0, &transfer_queue);
    }

    spdlog::debug("Created device successfully.");
}

Device::~Device() {
    vkDestroyDevice(this->device, nullptr);
    this->device = VK_NULL_HANDLE;

    queues_to_create.clear();
}

} // namespace inexor::vulkan_renderer::wrapper
