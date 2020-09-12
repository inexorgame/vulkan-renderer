#pragma once

#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer {

/// @brief Prints information related to graphics card's capabilities and limits.
class VulkanGraphicsCardInfoViewer {
public:
    VulkanGraphicsCardInfoViewer() = default;

    /// @brief
    void print_driver_vulkan_version();

    /// @brief
    /// @param graphics_card
    void print_physical_device_queue_families(const VkPhysicalDevice &graphics_card);

    /// @brief
    void print_instance_layers();

    /// @brief
    void print_instance_extensions();

    /// @brief
    /// @param graphics_card
    void print_device_layers(const VkPhysicalDevice &graphics_card);

    /// @brief
    /// @param graphics_card
    void print_device_extensions(const VkPhysicalDevice &graphics_card);

    /// @brief
    /// @param graphics_card
    /// @param vulkan_surface
    void print_surface_capabilities(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &vulkan_surface);

    /// @brief
    /// @param graphics_card
    /// @param vulkan_surface
    void print_supported_surface_formats(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &vulkan_surface);

    /// @brief
    /// @param graphics_card
    /// @param vulkan_surface
    void print_presentation_modes(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &vulkan_surface);

    /// @brief
    /// @param graphics_card
    void print_graphics_card_info(const VkPhysicalDevice &graphics_card);

    /// @brief
    /// @param graphics_card
    void print_graphics_card_limits(const VkPhysicalDevice &graphics_card);

    /// @brief
    /// @param graphics_card
    void print_graphics_cards_sparse_properties(const VkPhysicalDevice &graphics_card);

    /// @brief
    /// @param graphics_card
    void print_graphics_card_features(const VkPhysicalDevice &graphics_card);

    /// @brief
    /// @param graphics_card
    void print_graphics_card_memory_properties(const VkPhysicalDevice &graphics_card);

    /// @brief
    /// @param vulkan_instance
    /// @param vulkan_surface
    void print_all_physical_devices(const VkInstance &vulkan_instance, const VkSurfaceKHR &vulkan_surface);
};

} // namespace inexor::vulkan_renderer
