#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <cassert>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer {

/// @brief A RAII wrapper class for VkInstance.
/// @note The instantiation of this class must be synchronized externally.
class Instance {
protected:
    VkInstance instance = VK_NULL_HANDLE;

    std::uint32_t available_instance_layers = 0;
    std::uint32_t available_instance_extensions = 0;

    std::vector<VkLayerProperties> instance_layers_cache = {};
    std::vector<VkExtensionProperties> instance_extensions_cache = {};

    std::vector<const char *> enabled_instance_extensions = {};
    std::vector<const char *> enabled_instance_layers = {};

    /// When creating a VkInstance, we must check if the requested instance layers and instance extensions are supported.
    /// We therefore use instance_extension_available and instance_layer_available. In order to make those functions faster,
    /// we create a cache for all available instance extensions and instance layers. This way, looping through the list and
    /// comparing the entries is faster.
    /// @note This function is not thread safe.
    void create_availability_checks_cache();

    /// @brief Checks if a certain instance extension is available on the system.
    /// @param instance_extension_name [in] The name of the instance extension.
    /// @return True if the instance extension is available, false otherwise.
    bool instance_extension_available(const std::string &instance_extension_name);

    /// @brief Checks if a certain instance layer is available on the system.
    /// @param instance_layer_name [in]
    /// @return True if the instance layer is available, false otherwise.
    bool instance_layer_available(const std::string &instance_layer_name);

public:
    /// @brief Creates a VkInstance.
    Instance(const std::string &application_name, const std::string &engine_name, const std::uint32_t application_version, const std::uint32_t engine_version,
             const std::uint32_t vulkan_api_version, std::optional<std::vector<std::string>> requested_instance_extensions,
             std::optional<std::vector<std::string>> requested_instance_layers, bool enable_validation_layers, bool enable_renderdoc_layer);

    ~Instance();

    const VkInstance get_instance() const {
        return instance;
    }

    const std::vector<const char *> &get_enabled_instance_extensions() const {
        return enabled_instance_extensions;
    }

    const std::vector<const char *> &get_enabled_instance_layers() const {
        return enabled_instance_layers;
    }
};
} // namespace inexor::vulkan_renderer
