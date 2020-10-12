#pragma once

#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer {

/// @brief Print information related to graphics card's capabilities and limits.
struct VulkanGraphicsCardInfoViewer {

    /// @brief Query which version of the Vulkan API is supported on this system.
    /// @note Inexor engine does not use a Vulkan metaloader like volk.
    void print_driver_vulkan_version();

    /// @brief Print information about a graphics card's device queue families.
    /// @param graphics_card The regarded graphics card.
    void print_physical_device_queue_families(const VkPhysicalDevice &graphics_card);

    /// @brief Print which instance layers are available for the regarded graphics card on this system.
    void print_instance_layers();

    /// @brief Print which instance extensions are available for the regarded graphics card on this system.
    void print_instance_extensions();

    /// @brief Print which device layers are available for the regarded graphics card on this system.
    /// @param graphics_card The regarded graphics card.
    void print_device_layers(const VkPhysicalDevice &graphics_card);

    /// @brief Print which device extensions are available for the regarded graphics card on this system.
    /// @param graphics_card The regarded graphics card.
    void print_device_extensions(const VkPhysicalDevice &graphics_card);

    /// @brief Print supported surface capabilities of the regarded combination of graphics card and surface.
    /// @param graphics_card The regarded graphics card.
    /// @param vulkan_surface The regarded surface.
    void print_surface_capabilities(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &vulkan_surface);

    /// @brief Print supported surface formats of the regarded combination of graphics card and surface.
    /// @param graphics_card The regarded graphics card.
    // @param vulkan_surface The regarded Vulkan (window) surface.
    void print_supported_surface_formats(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &vulkan_surface);

    /// @brief List up all supported presentation modes.
    /// @param graphics_card The regarded graphics card.
    /// @param vulkan_surface The regarded surface.
    void print_presentation_modes(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &vulkan_surface);

    /// @brief Print the information about the graphics card.
    /// @param graphics_card The regarded graphics card.
    void print_graphics_card_info(const VkPhysicalDevice &graphics_card);

    /// @brief Print information about the limits of the graphics card.
    /// @param graphics_card The regarded graphics card.
    void print_graphics_card_limits(const VkPhysicalDevice &graphics_card);

    /// @brief Print information about the sparse properties of the graphics card.
    /// @param graphics_card The regarded graphics card.
    void print_graphics_cards_sparse_properties(const VkPhysicalDevice &graphics_card);

    /// @brief Print information about the features of the graphics card.
    /// @param graphics_card The regarded graphics card.
    void print_graphics_card_features(const VkPhysicalDevice &graphics_card);

    /// @brief Print information about the graphics card's memory properties.
    /// @param graphics_card The regarded graphics card.
    void print_graphics_card_memory_properties(const VkPhysicalDevice &graphics_card);

    /// @brief List up all available physical devices.
    /// @param vulkan_instance The instance of Vulkan.
    /// @param vulkan_surface The regarded Vulkan (window) surface.
    void print_all_physical_devices(const VkInstance &vulkan_instance, const VkSurfaceKHR &vulkan_surface);
};

} // namespace inexor::vulkan_renderer
