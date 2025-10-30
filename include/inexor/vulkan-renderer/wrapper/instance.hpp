#pragma once

#include <volk.h>

#include <cstdint>
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
/// @note **Design decision**: For 2 reasons, it was deliberately decided to avoid checking inside of the `Instance`
/// constructor which of the requested instance layers and instance extensions are available and only to enable those
/// which are available. **Reason 1**: This would require a class interface in which the programmer would have to check
/// which instance layer or instance extension is enabled **after** the `Instance` constructor has been called. This
/// would make using this wrapper more complex, and the programmer could even forget to do this and attempt to use
/// instance layer or instance extensions which are not available. It is not the responsibility of this class to do
/// this! In external code, **you** must check what is supported **before** calling this constructor and it is your
/// responsibility to store this data according to the needs of the application. **Reason 2**: If you pass any instance
/// layer or instance extension which is not supported by the system, an exception is thrown. We believe this follows
/// one of the most important design rules from Scott Meyers: Make APIs easy to use correctly and as hard to use
/// incorrectly.
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
    ///
    /// @warning **Make sure to understand**: It is the responsibility of the external code to ensure that all instance
    /// layers and all instance extensions which are passed into the constructor of the `Instance` class are actually
    /// available on the system! Use `is_layer_supported` and `is_extension_supported` to check which instance layers
    /// and instance extensions are available. This constructor will **not** check this again because if a single
    /// instance layer is not present, the creation with `vkCreateInstance` will fail and an exception will be thrown
    /// (`VK_ERROR_LAYER_NOT_PRESENT`). The same also applies if a single instance extension is not present, which will
    /// also result in an exception (`VK_ERROR_EXTENSION_NOT_PRESENT`). Furthermore, also make sure that any instance
    /// layer you pass really is an instance layer, not an instance extension or a device extension! Because all of
    /// these are just strings, they are easy to mix up. This also applies to instance extensions (or device extensions
    /// in the `Device` wrapper). In case you do mix them up here, the `Instance` wrapper will throw an exception
    /// (`VK_ERROR_LAYER_NOT_PRESENT` or `VK_ERROR_EXTENSION_NOT_PRESENT`, depending on what you mixed up).
    ///
    /// @note **Design decision**: You might be surprised to find out that we decided to check if
    /// `vkEnumerateInstanceVersion` and `vkCreateInstance` are valid function pointers. We do this because we load
    /// Vulkan dynamically with volk metaloader. The programmer might forget to call `volkInitialize` in external code
    /// before calling the constructor of the `Instance` class. If this was the case, both `vkEnumerateInstanceVersion`
    /// and `vkCreateInstance` would still be `nullptr`. The reason why this class is not calling `volkInitialize`
    /// internally is because `volkInitialize` will init those functions which are required for checking for available
    /// instance layers and instance extensions, which need to be checked **before** the constructor of the `Instance`
    /// class is called! So the order is: **1)** Call `volkInitialize` in external code, **2)** check for available
    /// instance layers and instance extensions, **3)** create an instance of the `Instance` class. In this
    /// constructor, after `vkCreateInstance` and subsequently `volkLoadInstanceOnly` have been called, we even check if
    /// `vkDestroyInstance` is a valid function pointer. We do this just to be sure, and it is strictly not necessary.
    /// If `vkDestroyInstance` would not be available after `volkLoadInstanceOnly`, there is either a bug in volk
    /// library or something is fundamentally wrong with the installed Vulkan runtime.
    Instance(const std::span<const char *> instance_layers = {},
             const std::span<const char *> instance_extensions = {});

    Instance(const Instance &) = delete;
    Instance(Instance &&) noexcept;

    ~Instance();

    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&) = delete;

    /// @note **Design decision**: Initially we thought we could make access to `m_instance` fully private and restrict
    /// access to it to some friend classes to avoid public access. However, we can't make access to the `VkInstance`
    /// private because several public functions require this.
    [[nodiscard]] VkInstance instance() const {
        return m_instance;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
