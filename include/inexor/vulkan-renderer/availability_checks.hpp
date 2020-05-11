#pragma once

#include "inexor/vulkan-renderer/error_handling.hpp"

#include <cassert>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer {

class AvailabilityChecksManager {
private:
    std::uint32_t available_instance_extensions = 0;

    std::uint32_t available_instance_layers = 0;

    std::uint32_t available_device_layers = 0;

    std::uint32_t available_device_extensions = 0;

    std::vector<VkExtensionProperties> instance_extensions_cache;

    std::vector<VkLayerProperties> instance_layers_cache;

    std::vector<VkLayerProperties> device_layer_properties_cache;

    std::vector<VkExtensionProperties> device_extensions_cache;

    VkResult create_instance_layers_cache();

    VkResult create_device_layers_cache(const VkPhysicalDevice &graphics_card);

    VkResult create_device_extensions_cache(const VkPhysicalDevice &graphics_card);

    VkResult create_instance_extensions_cache();

public:
    AvailabilityChecksManager() = default;

    ~AvailabilityChecksManager() = default;

    /// @brief Checks if a certain Vulkan instance layer is available on the system.
    /// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateInstanceLayerProperties.html
    /// @param instance_layer_name The name of the Vulkan instance layer.
    /// @return true if the Vulkan instance layer is available, false otherwise.
    /// @note Available instance layers can be enabled by passing them as parameter during Vulkan instance creation.
    [[nodiscard]] bool has_instance_layer(const std::string &instance_layer_name);

    /// @brief Checks if a certain Vulkan instance extension is available on the system.
    /// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateInstanceExtensionProperties.html
    /// @param instance_extension_name The name of the Vulkan instance extension.
    /// @return true if the Vulkan instance extension is available, false otherwise.
    /// @note Available instance extensions can be enabled by passing them as parameter during Vulkan instance creation.
    [[nodiscard]] bool has_instance_extension(const std::string &instance_extension_name);

    /// @brief Checks if a certain Vulkan device layer is available on the system.
    /// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateDeviceLayerProperties.html
    /// @note Device layers and device extensions are coupled to a certain graphics card which needs to be specified as parameter.
    /// @param graphics_card The selected graphics card.
    /// @param device_layer_name The name of the Vulkan device layer.
    /// @return true if the Vulkan device layer is available, false otherwise.
    /// @note Available device layers can be enabled by passing them as a parameter during Vulkan device creation.
    [[nodiscard]] bool has_device_layer(const VkPhysicalDevice &graphics_card, const std::string &device_layer_name);

    /// @brief Checks if a certain Vulkan device extension is available on the system.
    /// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateDeviceExtensionProperties.html
    /// @param graphics_card [in] The selected graphics card.
    /// @param device_extension_name [in] The name of the Vulkan device extension.
    /// @return true if the Vulkan device extension is available, false otherwise.
    /// @note Available device extensions can be enabled by passing them as a parameter during Vulkan device creation.
    /// @note Device layers and device extensions are coupled to a certain graphics card which needs to be specified as parameter.
    [[nodiscard]] bool has_device_extension(const VkPhysicalDevice &graphics_card, const std::string &device_extension_name);

    /// @brief Checks if presentation is available for a certain combination of graphics card and window surface.
    /// The present mode describes how the rendered image will be presented on the screen.
    /// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
    /// @param graphics_card [in] The selected graphics card.
    /// @param surface [surface] The window surface.
    /// @return true if presentation is available, false otherwise.
    [[nodiscard]] bool has_presentation(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface);

    /// @brief Checks if swapchain is available for a certain graphics card.
    /// @param graphics_card [in] The selected graphics card.
    /// @return true if swapchain is available, false otherwise.
    [[nodiscard]] bool has_swapchain(const VkPhysicalDevice &graphics_card);
};

} // namespace inexor::vulkan_renderer
