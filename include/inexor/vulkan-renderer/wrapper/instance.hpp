#pragma once

#include "inexor/vulkan-renderer/availability_checks.hpp"

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

/// @class Instance
/// @brief RAII wrapper class for VkInstances.
class Instance {
protected:
    VkInstance m_instance{VK_NULL_HANDLE};
    AvailabilityChecksManager m_availability_checks;

public:
    /// @brief Constructs the Vulkan instance and specifies the requested instance layers and instance extensions.
    /// @param application_name [in] The Vulkan application's internal application name.
    /// @param engine_name [in] The Vulkan appliation's internal engine name.
    /// @param application_version [in] The Vulkan application's internal version.
    /// @param engine_version [in] The Vulkan application's internal engine version.
    /// @param vulkan_api_version [in] The requested version of Vulkan API from which an instance will be created.
    /// If the requested version of Vulkan API is not available, the creation of the instance will fail!
    /// @param enable_validation_layers [in] True if validation layers should be enabled, false otherwise.
    /// @param enable_renderdoc_layer [in] True if renderdoc layer should be enabled, false otherwise.
    /// @param requested_instance_extensions [in] The instance extensions which are requested.
    /// @param requested_instance_layers [in] The instance layers which are requested.
    Instance(const std::string &application_name, const std::string &engine_name,
             const std::uint32_t application_version, const std::uint32_t engine_version,
             const std::uint32_t vulkan_api_version, bool enable_validation_layers, bool enable_renderdoc_layer,
             std::vector<std::string> requested_instance_extensions,
             std::vector<std::string> requested_instance_layers);

    /// @brief Constructs the Vulkan instance without the requested instance layers and instance extensions.
    /// @param application_name [in] The Vulkan application's internal application name.
    /// @param engine_name [in] The Vulkan appliation's internal engine name.
    /// @param application_version [in] The Vulkan application's internal version.
    /// @param engine_version [in] The Vulkan application's internal engine version.
    /// @param vulkan_api_version [in] The requested version of Vulkan API from which an instance will be created.
    /// If the requested version of Vulkan API is not available, the creation of the instance will fail!
    /// @param enable_validation_layers [in] True if validation layers should be enabled, false otherwise.
    /// @param enable_renderdoc_layer [in] True if renderdoc layer should be enabled, false otherwise.
    Instance(const std::string &application_name, const std::string &engine_name,
             const std::uint32_t application_version, const std::uint32_t engine_version,
             const std::uint32_t vulkan_api_version, bool enable_validation_layers, bool enable_renderdoc_layer);
    Instance(const Instance &) = delete;
    Instance(Instance &&) noexcept;
    ~Instance();

    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) = default;

    [[nodiscard]] VkInstance instance() const {
        return m_instance;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
