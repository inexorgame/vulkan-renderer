#pragma once

#include <volk.h>

#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// Check if a certain instance extension is supported on the system.
/// @note If is the responsibility of the programmer to make sure that every instance layer which is passed to the
/// constructor of `Instance` is actually available on the system *before* calling the constructor!
/// @param extension_name The name of the instance extension.
/// @return `true` if the instance extension is supported.
/// @throws InexorException if `extension_name` is not an instance extension, but an instance layer.
/// @throws InexorException if `vkEnumerateInstanceExtensionProperties` is `nullptr`.
/// @throws VulkanException if `vkEnumerateInstanceExtensionProperties` does not return `VK_SUCCESS`.
[[nodiscard]] bool is_instance_extension_supported(const std::string &extension_name);

/// Check if a certain instance layer is available on the system.
/// @note If is the responsibility of the programmer to make sure that every instance layer which is passed to the
/// constructor of `Instance` is actually available on the system *before* calling the constructor!
/// @param layer_name The name of the instance layer.
/// @return `true` if the instance layer is supported.
/// @throws InexorException if `layer_name` is not an instance layer, but an instance extension.
/// @throws InexorException if vkEnumerateInstanceLayerProperties is `nullptr`.
/// @throws VulkanException if `vkEnumerateInstanceLayerProperties` does not return `VK_SUCCESS`.
[[nodiscard]] bool is_instance_layer_supported(const std::string &layer_name);

/// RAII wrapper class for `VkInstance`.
/// @warning It is the responsibility of the programmer to ensure that all instance layers and all instance
/// extensions which are passed into this constructor are actually available on the system! Use the static methods
/// `is_layer_supported` and `is_extension_supported` to check if instance layers and extensions are available.
/// This constructor will not check this again because if a single instance layer is not present, the creation will
/// fail and an exception will be thrown (`VK_ERROR_LAYER_NOT_PRESENT`). The same also applies if a single
/// instance extension is not present, which will also result in an exception (`VK_ERROR_EXTENSION_NOT_PRESENT`).
/// It was deliberately decided to avoid checking which instance layers or instance extensions are available and
/// only to enable those which are available, because this would require an interface in which the programmer would
/// have to check which instance layer or instance extension is enabled after the constructor has been called. This
/// would make this API more complex, and it is not the responsibility of this class to do this! You must check what
/// is supported before calling this constructor.
class Instance {
private:
    VkInstance m_instance{VK_NULL_HANDLE};

public:
    /// This is the version of Vulkan API that we use in the entire engine.
    static constexpr std::uint32_t REQUIRED_VK_API_VERSION{VK_API_VERSION_1_2};

    /// Default constructor.
    /// @param instance_layers The required instance layers (can be empty).
    /// @param instance_extensions The required instance extensions (can be empty).
    /// @throws InexorException if function pointer `vkEnumerateInstanceVersion` is not available.
    /// @throws InexorException if function pointer `vkCreateInstance` is not available.
    /// @throws VulkanException if `vkEnumerateInstanceVersion` does not return `VK_SUCCESS`.
    /// @throws VulkanException if `vkCreateInstance` does not return `VK_SUCCESS`.
    /// @throws InexorException if function pointer `vkDestroyInstance` is not available after `volkLoadInstanceOnly`.
    Instance(const std::span<const char *> instance_layers = {},
             const std::span<const char *> instance_extensions = {});

    Instance(const Instance &) = delete;
    Instance(Instance &&) noexcept;

    ~Instance();

    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) = delete;

    /// @note We can't make access to the `VkInstance` private because even some public functions require this.
    [[nodiscard]] VkInstance instance() const {
        return m_instance;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
