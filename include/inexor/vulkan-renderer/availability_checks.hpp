#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer {

class AvailabilityChecksManager {
private:
    std::uint32_t m_available_instance_extensions{0};
    std::uint32_t m_available_instance_layers{0};
    std::uint32_t m_available_device_layers{0};
    std::uint32_t m_available_device_extensions{0};

    std::vector<VkExtensionProperties> m_instance_extensions_cache;
    std::vector<VkLayerProperties> m_instance_layers_cache;
    std::vector<VkLayerProperties> m_device_layer_properties_cache;
    std::vector<VkExtensionProperties> m_device_extensions_cache;

    void create_instance_layers_cache();
    void create_device_layers_cache(const VkPhysicalDevice &graphics_card);
    void create_device_extensions_cache(const VkPhysicalDevice &graphics_card);
    void create_instance_extensions_cache();

public:
    AvailabilityChecksManager() = default;

    /// @brief Checks if a certain Vulkan instance layer is available on the system.
    /// @note Available instance layers can be enabled by passing them as parameter during Vulkan instance creation.
    /// @param instance_layer_name [in] The name of the Vulkan instance layer.
    /// @return true if the Vulkan instance layer is available, false otherwise.
    [[nodiscard]] bool has_instance_layer(const std::string &instance_layer_name);

    /// @brief Checks if a certain Vulkan instance extension is available on the system.
    /// @note Available instance extensions can be enabled by passing them as parameter during Vulkan instance creation.
    /// @param instance_extension_name [in] The name of the Vulkan instance extension.
    /// @return True if the Vulkan instance extension is available, false otherwise.
    [[nodiscard]] bool has_instance_extension(const std::string &instance_extension_name);

    /// @brief Checks if a certain Vulkan device layer is available on the system.
    /// @note Device layers and device extensions are coupled to a certain graphics card which needs to be specified as
    /// parameter. Available device layers can be enabled by passing them as a parameter during Vulkan device creation.
    /// @param graphics_card [in] The selected graphics card.
    /// @param device_layer_name [in] The name of the Vulkan device layer.
    /// @return True if the Vulkan device layer is available, false otherwise.
    [[nodiscard]] bool has_device_layer(const VkPhysicalDevice &graphics_card, const std::string &device_layer_name);

    /// @brief Checks if a certain Vulkan device extension is available on the system.
    /// Available device extensions can be enabled by passing them as a parameter during Vulkan device creation.
    /// @param graphics_card [in] The selected graphics card.
    /// @param device_extension_name [in] The name of the Vulkan device extension.
    /// @return True if the Vulkan device extension is available, false otherwise.
    [[nodiscard]] bool has_device_extension(const VkPhysicalDevice &graphics_card,
                                            const std::string &device_extension_name);

    /// @brief Checks if presentation is available for a certain combination of graphics card and window surface.
    /// The present mode describes how the rendered image will be presented on the screen.
    /// @param graphics_card [in] The selected graphics card.
    /// @param surface [surface] The window surface.
    /// @return True if presentation is available, false otherwise.
    [[nodiscard]] bool has_presentation(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface);

    /// @brief Checks if swapchain is available for a certain graphics card.
    /// @param graphics_card [in] The selected graphics card.
    /// @return true if swapchain is available, false otherwise.
    [[nodiscard]] bool has_swapchain(const VkPhysicalDevice &graphics_card);
};

} // namespace inexor::vulkan_renderer
