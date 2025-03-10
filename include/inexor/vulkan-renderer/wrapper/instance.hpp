#pragma once

#include <volk.h>

#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;
class Surface;

/// RAII wrapper class for VkInstance and VkDebugUtilsMessengerEXT
class Instance {
    friend class Device;
    friend class Surface;

private:
    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debug_callback{VK_NULL_HANDLE};

public:
    /// Construct the Vulkan instance and specify the requested instance layers and instance extensions
    /// @param application_name The Vulkan application's internal application name
    /// @param engine_name The Vulkan application's internal engine name
    /// @param application_version The Vulkan application's internal version
    /// @param engine_version The Vulkan application's internal engine version
    /// @param debug_callback The debug utils messenger callback (VK_EXT_debug_utils)
    /// @param requested_instance_extensions The instance extensions which are requested (empty by default)
    /// @param requested_instance_layers The instance layers which are requested (empty by default)
    Instance(const std::string &application_name,
             const std::string &engine_name,
             std::uint32_t application_version,
             std::uint32_t engine_version,
             PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
             const std::vector<std::string> &requested_instance_extensions = {},
             const std::vector<std::string> &requested_instance_layers = {});

    Instance(const Instance &) = delete;
    Instance(Instance &&) noexcept;

    /// Call vkDestroyDebugUtilsMessengerEXT and vkDestroyInstance
    ~Instance();

    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) noexcept;

    /// Check if a certain instance layer is available on the system
    /// @param layer_name The name of the instance layer
    /// @return ``true`` if the instance layer is supported
    [[nodiscard]] static bool is_layer_supported(const std::string &layer_name);

    /// Check if a certain instance extension is supported on the system
    /// @param extension_name The name of the instance extension
    /// @return ``true`` if the instance extension is supported
    [[nodiscard]] static bool is_extension_supported(const std::string &extension_name);

    /// Return all physical devices of the system
    /// @return The available physical devices
    [[nodiscard]] std::vector<VkPhysicalDevice> get_physical_devices() const;

    /// Use Vulkan 1.3
    static constexpr std::uint32_t REQUIRED_VK_API_VERSION{VK_API_VERSION_1_3};
};

} // namespace inexor::vulkan_renderer::wrapper
