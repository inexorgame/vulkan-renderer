#include "inexor/vulkan-renderer/gpu_queue_manager.hpp"

namespace inexor::vulkan_renderer {

VkResult VulkanQueueManager::init(const std::shared_ptr<VulkanSettingsDecisionMaker> settings_decision_maker) {
    // TODO: Add mutex.
    this->settings_decision_maker = settings_decision_maker;

    queue_manager_initialised = true;

    return VK_SUCCESS;
}

VkResult VulkanQueueManager::setup_queues(const VkDevice &device) {
    assert(queue_manager_initialised);
    assert(present_queue_family_index.has_value());
    assert(graphics_queue_family_index.has_value());
    assert(data_transfer_queue_family_index.has_value());

    spdlog::debug("Initialising GPU queues.");

    spdlog::debug("Graphics queue family index: {}.", graphics_queue_family_index.value());
    spdlog::debug("Presentation queue family index: {}.", present_queue_family_index.value());
    spdlog::debug("Data transfer queue family index: {}.", data_transfer_queue_family_index.value());

    // Setup the queues for presentation and graphics.
    // Since we only have one queue per queue family, we acquire index 0.
    vkGetDeviceQueue(device, present_queue_family_index.value(), 0, &present_queue);
    vkGetDeviceQueue(device, graphics_queue_family_index.value(), 0, &graphics_queue);

    // The use of data transfer queues can be forbidden by using -no_separate_data_queue.
    if (use_distinct_data_transfer_queue && data_transfer_queue_family_index.has_value()) {
        // Use a separate queue for data transfer to GPU.
        vkGetDeviceQueue(device, data_transfer_queue_family_index.value(), 0, &data_transfer_queue);
    }

    return VK_SUCCESS;
}

VkResult VulkanQueueManager::prepare_queues(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface,
                                            bool use_distinct_data_transfer_queue_if_available) {
    assert(queue_manager_initialised);
    assert(graphics_card);

    spdlog::debug("Creating Vulkan device queues.");

    if (use_distinct_data_transfer_queue_if_available) {
        spdlog::debug("The application will try to use a distinct data transfer queue if it is available.");
    } else {
        spdlog::warn("The application is forced not to use a distinct data transfer queue!");
    }

    // This is neccesary since device queues might be recreated as swapchain becomes invalid.
    device_queues_to_create.clear();

    // Check if there is one queue family which can be used for both graphics and presentation.
    std::optional<std::uint32_t> queue_family_index_for_both_graphics_and_presentation =
        settings_decision_maker->find_queue_family_for_both_graphics_and_presentation(graphics_card, surface);

    if (queue_family_index_for_both_graphics_and_presentation.has_value()) {
        spdlog::debug("One queue for both graphics and presentation will be used.");

        graphics_queue_family_index = queue_family_index_for_both_graphics_and_presentation.value();

        present_queue_family_index = graphics_queue_family_index;

        use_one_queue_family_for_graphics_and_presentation = true;

        // In this case, there is one queue family which can be used for both graphics and presentation.
        VkDeviceQueueCreateInfo device_queue_create_info = {};

        // For now, we only need one queue family.
        std::uint32_t number_of_combined_queues_to_use = 1;

        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.pNext = nullptr;
        device_queue_create_info.flags = 0;
        device_queue_create_info.queueFamilyIndex = queue_family_index_for_both_graphics_and_presentation.value();
        device_queue_create_info.queueCount = number_of_combined_queues_to_use;
        device_queue_create_info.pQueuePriorities = &global_queue_priority;

        device_queues_to_create.push_back(device_queue_create_info);
    } else {
        spdlog::debug("No queue found which supports both graphics and presentation.");
        spdlog::debug("The application will try to use 2 separate queues.");

        // We have to use 2 different queue families.
        // One for graphics and another one for presentation.

        // Check which queue family index can be used for graphics.
        graphics_queue_family_index = settings_decision_maker->find_graphics_queue_family(graphics_card);

        if (!graphics_queue_family_index.has_value()) {
            spdlog::critical("Could not find suitable queue family indices for graphics!");
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Check which queue family index can be used for presentation.
        present_queue_family_index = settings_decision_maker->find_presentation_queue_family(graphics_card, surface);

        if (!present_queue_family_index.has_value()) {
            spdlog::critical("Could not find suitable queue family indices for presentation!");
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        spdlog::debug("Graphics queue family index: {}.", graphics_queue_family_index.value());
        spdlog::debug("Presentation queue family index: {}.", present_queue_family_index.value());

        // Set up one queue for graphics.
        VkDeviceQueueCreateInfo device_queue_create_info_for_graphics_queue = {};

        // For now, we only need one queue family.
        std::uint32_t number_of_graphics_queues_to_use = 1;

        device_queue_create_info_for_graphics_queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info_for_graphics_queue.pNext = nullptr;
        device_queue_create_info_for_graphics_queue.flags = 0;
        device_queue_create_info_for_graphics_queue.queueFamilyIndex = graphics_queue_family_index.value();
        device_queue_create_info_for_graphics_queue.queueCount = number_of_graphics_queues_to_use;
        device_queue_create_info_for_graphics_queue.pQueuePriorities = &global_queue_priority;

        device_queues_to_create.push_back(device_queue_create_info_for_graphics_queue);

        // Set up one queue for presentation.
        VkDeviceQueueCreateInfo device_queue_create_info_for_presentation_queue = {};

        // For now, we only need one queue family.
        std::uint32_t number_of_present_queues_to_use = 1;

        device_queue_create_info_for_presentation_queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info_for_presentation_queue.pNext = nullptr;
        device_queue_create_info_for_presentation_queue.flags = 0;
        device_queue_create_info_for_presentation_queue.queueFamilyIndex = present_queue_family_index.value();
        device_queue_create_info_for_presentation_queue.queueCount = number_of_present_queues_to_use;
        device_queue_create_info_for_presentation_queue.pQueuePriorities = &global_queue_priority;

        device_queues_to_create.push_back(device_queue_create_info_for_presentation_queue);
    }

    // Add another device queue just for data transfer.
    data_transfer_queue_family_index = settings_decision_maker->find_distinct_data_transfer_queue_family(graphics_card);

    if (data_transfer_queue_family_index.has_value() && use_distinct_data_transfer_queue_if_available) {
        spdlog::debug("A separate queue will be used for data transfer.");
        spdlog::debug("Data transfer queue family index: {}.", data_transfer_queue_family_index.value());

        // We have the opportunity to use a separated queue for data transfer!
        use_distinct_data_transfer_queue = true;

        VkDeviceQueueCreateInfo device_queue_for_data_transfer_create_info = {};

        // For now, we only need one queue family.
        std::uint32_t number_of_queues_to_use = 1;

        device_queue_for_data_transfer_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_for_data_transfer_create_info.pNext = nullptr;
        device_queue_for_data_transfer_create_info.flags = 0;
        device_queue_for_data_transfer_create_info.queueFamilyIndex = data_transfer_queue_family_index.value();
        device_queue_for_data_transfer_create_info.queueCount = number_of_queues_to_use;
        device_queue_for_data_transfer_create_info.pQueuePriorities = &global_queue_priority;

        device_queues_to_create.push_back(device_queue_for_data_transfer_create_info);
    } else {
        // We don't have the opportunity to use a separated queue for data transfer!
        // Do not create a new queue, use the graphics queue instead.
        use_distinct_data_transfer_queue = false;
    }

    if (!use_distinct_data_transfer_queue) {
        spdlog::warn("The application is forces to avoid distinct data transfer queues.");
        spdlog::warn("Because of this, the graphics queue will be used for data transfer.");

        data_transfer_queue_family_index = graphics_queue_family_index;
    }

    return VK_SUCCESS;
}

VkResult VulkanQueueManager::prepare_swapchain_creation(VkSwapchainCreateInfoKHR &swapchain_create_info) {
    assert(queue_manager_initialised);

    if (use_one_queue_family_for_graphics_and_presentation) {
        // In this case, we can use one queue family for both graphics and presentation.
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    } else {
        // In this case, we can't use the same queue family for both graphics and presentation.
        // We must use 2 separate queue families!
        const std::vector<std::uint32_t> queue_family_indices = {graphics_queue_family_index.value(), present_queue_family_index.value()};

        // It is important to note that we can't use VK_SHARING_MODE_EXCLUSIVE in this case.
        // VK_SHARING_MODE_CONCURRENT may result in lower performance access to the buffer or image than VK_SHARING_MODE_EXCLUSIVE.
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices.data();
        swapchain_create_info.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_family_indices.size());
    }

    return VK_SUCCESS;
}

VkQueue VulkanQueueManager::get_graphics_queue() {
    assert(queue_manager_initialised);
    assert(graphics_queue);

    return graphics_queue;
}

VkQueue VulkanQueueManager::get_present_queue() {
    assert(queue_manager_initialised);
    assert(present_queue);

    return present_queue;
}

VkQueue VulkanQueueManager::get_data_transfer_queue() {
    assert(queue_manager_initialised);

    if (use_distinct_data_transfer_queue) {
        assert(data_transfer_queue);
        return data_transfer_queue;
    }

    // Otherwise just use the graphics queue.
    assert(graphics_queue);
    return graphics_queue;
}

std::optional<std::uint32_t> VulkanQueueManager::get_graphics_family_index() {
    assert(queue_manager_initialised);
    assert(graphics_queue_family_index.has_value());

    return graphics_queue_family_index;
}

std::optional<std::uint32_t> VulkanQueueManager::get_present_queue_family_index() {
    assert(queue_manager_initialised);
    assert(present_queue_family_index.has_value());

    return present_queue_family_index;
}

std::optional<std::uint32_t> VulkanQueueManager::get_data_transfer_queue_family_index() {
    assert(queue_manager_initialised);
    assert(data_transfer_queue_family_index.has_value());

    return data_transfer_queue_family_index;
}

std::vector<VkDeviceQueueCreateInfo> VulkanQueueManager::get_queues_to_create() {
    assert(queue_manager_initialised);
    assert(!device_queues_to_create.empty());

    return device_queues_to_create;
}

} // namespace inexor::vulkan_renderer
