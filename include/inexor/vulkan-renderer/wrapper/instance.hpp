#pragma once

#include <volk.h>

#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

/// @brief RAII wrapper class for VkInstances.
class Instance {
private:
    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debug_callback{VK_NULL_HANDLE};

public:
    // This is the version of Vulkan API that we use.
    static constexpr std::uint32_t REQUIRED_VK_API_VERSION{VK_API_VERSION_1_2};

    /// @brief Construct the Vulkan instance and specify the requested instance layers and instance extensions.
    /// @param application_name The Vulkan application's internal application name
    /// @param engine_name The Vulkan application's internal engine name
    /// @param application_version The Vulkan application's internal version
    /// @param engine_version The Vulkan application's internal engine version
    /// @param debug_callback The debug callback (``VK_EXT_debug_utils``)
    /// @param enable_validation_layers True if validation layers should be enabled
    /// @param requested_instance_extensions The instance extensions which are requested
    /// @param requested_instance_layers The instance layers which are requested
    Instance(const std::string &application_name, const std::string &engine_name, std::uint32_t application_version,
             std::uint32_t engine_version, const PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
             bool enable_validation_layers, const std::vector<std::string> &requested_instance_extensions,
             const std::vector<std::string> &requested_instance_layers);

    /// @brief Construct the Vulkan instance without the requested instance layers and instance extensions.
    /// @param application_name The Vulkan application's internal application name
    /// @param engine_name The Vulkan application's internal engine name
    /// @param application_version The Vulkan application's internal version
    /// @param engine_version The Vulkan application's internal engine version
    /// @param enable_validation_layers True if validation layers should be enabled, false otherwise
    Instance(const std::string &application_name, const std::string &engine_name, std::uint32_t application_version,
             std::uint32_t engine_version, const PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
             bool enable_validation_layers);

    Instance(const Instance &) = delete;
    Instance(Instance &&) noexcept;

    ~Instance();

    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) = delete;

    [[nodiscard]] VkInstance instance() const {
        return m_instance;
    }

    /// @brief Check if a certain instance extension is supported on the system.
    /// @param extension_name The name of the instance extension.
    /// @return ``true`` if the instance extension is supported.
    [[nodiscard]] static bool is_extension_supported(const std::string &extension_name);

    /// @brief Check if a certain instance layer is available on the system.
    /// @param layer_name The name of the instance layer.
    /// @return ``true`` if the instance layer is supported.
    [[nodiscard]] static bool is_layer_supported(const std::string &layer_name);
};

} // namespace inexor::vulkan_renderer::wrapper
