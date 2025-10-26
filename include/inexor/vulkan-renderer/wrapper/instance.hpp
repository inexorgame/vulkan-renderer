#pragma once

#include <volk.h>

#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// RAII wrapper class for VkInstance
class Instance {
private:
    VkInstance m_instance{VK_NULL_HANDLE};

public:
    // This is the version of Vulkan API that we use in the entire engine.
    static constexpr std::uint32_t REQUIRED_VK_API_VERSION{VK_API_VERSION_1_2};

    /// Default constructor
    /// @param app_name The app name.
    /// @param engine_name The engine name.
    /// @param app_version The app version.
    /// @param engine_version The engine version.
    /// @param required_extensions The required instance extensions (empty by default).
    /// @param required_layers The required instance layers (empty by default).
    Instance(const std::string &app_name, const std::string &engine_name, std::uint32_t app_version,
             std::uint32_t engine_version, const std::span<const char *> required_extensions = {},
             const std::span<const char *> required_layers = {});

    Instance(const Instance &) = delete;
    Instance(Instance &&) noexcept;

    ~Instance();

    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) = delete;

    /// We can't make access to the VkInstance private because even some public functions require this.
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
