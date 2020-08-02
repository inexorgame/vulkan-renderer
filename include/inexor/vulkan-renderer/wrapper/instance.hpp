#pragma once

#include "inexor/vulkan-renderer/availability_checks.hpp"

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

/// @brief A RAII wrapper class for VkInstance.
/// @note The instantiation of this class must be synchronized externally.
class Instance {
protected:
    VkInstance m_instance = VK_NULL_HANDLE;
    AvailabilityChecksManager m_availability_checks;

public:
    /// Delete the copy constructor so Vulkan instances are move-only objects.
    Instance(const Instance &) = delete;
    Instance(Instance &&other) noexcept;

    /// Delete the copy assignment operator so Vulkan instances are move-only objects.
    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) noexcept = default;

    /// @brief Creates a VkInstance.
    /// @param application_name [in] The name of the application.
    /// @param engine_name [in] The name of the engine.
    /// @param application_version [in] The version of the application.
    /// @param engine_version [in] The version of the engine.
    /// @param vulkan_api_version [in] The requested version of Vulkan API.
    /// @note In Vulkan API we can use the VK_MAKE_VERSION() macro to create a std::uint32_t value from major, minor and
    /// patch number.
    /// @param requested_instance_extensions [in] A vector of required Vulkan instance extensions.
    /// @param requested_instance_layers [in] A vector of required Vulkan instance layers.
    /// @param enable_validation_layers [in] True if validation layers are requested, false otherwise.
    /// @param enable_renderdoc_layer [in] True if RenderDoc instance layer is requested, false otherwise.
    Instance(const std::string &application_name, const std::string &engine_name,
             const std::uint32_t application_version, const std::uint32_t engine_version,
             const std::uint32_t vulkan_api_version, std::vector<std::string> requested_instance_extensions,
             std::vector<std::string> requested_instance_layers, bool enable_validation_layers,
             bool enable_renderdoc_layer);

    /// @brief Creates a VkInstance.
    /// @note When this constructor is used, no instance layers or instance extensions will be requested,
    /// Vulkan validation layers will be requested, and RenderDoc instance layer will not be requested.
    /// @param application_name [in] The name of the application.
    /// @param engine_name [in] The name of the engine.
    /// @param application_version [in] The version of the application.
    /// @param engine_version [in] The version of the engine.
    /// @param vulkan_api_version [in] The requested version of Vulkan API.
    /// @note In Vulkan API we can use the VK_MAKE_VERSION() macro to create a std::uint32_t value from major, minor and
    /// patch number.
    Instance(const std::string &application_name, const std::string &engine_name,
             const std::uint32_t application_version, const std::uint32_t engine_version,
             const std::uint32_t vulkan_api_version);

    ~Instance();

    [[nodiscard]] const VkInstance get_instance() const {
        return m_instance;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
