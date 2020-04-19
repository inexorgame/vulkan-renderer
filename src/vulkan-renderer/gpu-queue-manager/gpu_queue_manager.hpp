#pragma once

#include <vulkan/vulkan.h>

#include <optional>

#include <cassert>

#include <spdlog/spdlog.h>

#include "vulkan-renderer/settings-decision-maker/settings_decision_maker.hpp"

namespace inexor {
namespace vulkan_renderer {

class VulkanQueueManager {
public:
    VulkanQueueManager() = default;

    ~VulkanQueueManager() = default;

private:
    //
    bool use_one_queue_family_for_graphics_and_presentation = false;

    //
    bool use_distinct_data_transfer_queue = true;

    //
    std::optional<uint32_t> graphics_queue_family_index = std::nullopt;

    //
    std::optional<uint32_t> present_queue_family_index = std::nullopt;

    //
    std::optional<uint32_t> data_transfer_queue_family_index = std::nullopt;

    //
    VkQueue graphics_queue = VK_NULL_HANDLE;

    //
    VkQueue present_queue = VK_NULL_HANDLE;

    //
    VkQueue data_transfer_queue = VK_NULL_HANDLE;

    //
    const float global_queue_priority = 1.0f;

    //
    std::vector<VkDeviceQueueCreateInfo> device_queues_to_create;

    bool queue_manager_initialised = false;

    VkDevice device;

    VkPhysicalDevice graphics_card;

    std::shared_ptr<VulkanSettingsDecisionMaker> settings_decision_maker;

public:
    /// @brief Initialises Vulkan queue manager.
    /// @param
    VkResult initialise(const std::shared_ptr<VulkanSettingsDecisionMaker> settings_decision_maker);

    ///
    VkResult setup_queues(const VkDevice &device);

    ///
    VkResult prepare_queues(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface, bool use_distinct_data_transfer_queue_if_available);

    ///
    VkResult prepare_swapchain_creation(VkSwapchainCreateInfoKHR &swapchain_create_info);

    ///
    VkQueue get_graphics_queue();

    ///
    VkQueue get_present_queue();

    ///
    VkQueue get_data_transfer_queue();

    ///
    std::optional<uint32_t> get_graphics_family_index();

    ///
    std::optional<uint32_t> get_present_queue_family_index();

    ///
    std::optional<uint32_t> get_data_transfer_queue_family_index();

    ///
    std::vector<VkDeviceQueueCreateInfo> get_queues_to_create();
};

}; // namespace vulkan_renderer
}; // namespace inexor
