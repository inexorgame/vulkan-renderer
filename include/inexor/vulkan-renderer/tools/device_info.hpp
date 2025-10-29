#pragma once

#include <volk.h>

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Instance;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::tools {

// Using declaration
using wrapper::Instance;

/// A wrapper struct for physical device data.
struct DeviceInfo {
    std::string name;
    VkPhysicalDevice physical_device{nullptr};
    VkPhysicalDeviceType type{VK_PHYSICAL_DEVICE_TYPE_OTHER};
    VkDeviceSize total_device_local{0};
    VkPhysicalDeviceFeatures features{};
    std::vector<VkExtensionProperties> extensions;
    bool presentation_supported{false};
    bool swapchain_supported{false};
};

/// Build DeviceInfo from a real vulkan physical device (as opposed to a fake one used in the tests).
/// @param physical_device The physical device
/// @param surface The window surface
/// @return The device info data
[[nodiscard]] DeviceInfo build_device_info(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

/// Compare two physical devices and determine which one is preferable
/// @param required_features The required device features which must all be supported by the physical device
/// @param required_extensions The required device extensions which must all be supported by the physical device
/// @param lhs A physical device to compare with the other one
/// @param rhs The other physical device
/// @return ``true`` if `lhs` is more preferable over `rhs`
[[nodiscard]] bool compare_physical_devices(VkPhysicalDeviceFeatures &required_features,
                                            std::span<const char *> required_extensions, const DeviceInfo &lhs,
                                            const DeviceInfo &rhs);

/// A function for rating physical devices by type
/// @param info The physical device info
/// @return A number from 0 to 2 which rates the physical device (higher is better)
[[nodiscard]] std::uint32_t device_type_rating(const DeviceInfo &info);

/// Transform a ``VkPhysicalDeviceFeatures`` into a ``std::vector<VkBool32>``
/// @note The size of the vector will be determined by the number of ``VkBool32`` variables in the
/// ``VkPhysicalDeviceFeatures`` struct
/// @param features The physical device features
/// @return A ``std::vector<VkBool32>`` The transformed data
[[nodiscard]] std::vector<VkBool32> get_device_features_as_vector(const VkPhysicalDeviceFeatures &features);

/// Get the name of a physical device
/// @param physical_device The physical device
/// @return The name of the physical device
[[nodiscard]] std::string get_physical_device_name(VkPhysicalDevice physical_device);

/// Determine if a physical device is suitable. In order for a physical device to be suitable,
/// it must support all required device features and required extensions.
/// @param info The device info data
/// @param required_features The required device features the physical device must all support
/// @param required_extensions The required device extensions the physical device must all support
/// @param print_message If ``true``, an info message will be printed to the console if a device feature or device
/// extension is not supported (``true`` by default)
/// @return ``true`` if the physical device supports all device features and device extensions
[[nodiscard]] bool is_gpu_suitable(const DeviceInfo &info, const VkPhysicalDeviceFeatures &required_features,
                                   const std::span<const char *> required_extensions, bool print_info = false);

/// Check if a device extension is supported by a physical device
/// @param extensions The device extensions
/// @note If extensions is empty, this function returns ``false``
/// @param extension_name The extension name
/// @return ``true`` if the required device extension is supported
[[nodiscard]] bool is_extension_supported(const std::vector<VkExtensionProperties> &extensions,
                                          const std::string &extension_name);

/// Pick the best physical device automatically
/// @param physical_device_infos The data of the physical devices
/// @param required_features The required device features
/// @param required_extensions The required device extensions
/// @exception std::runtime_error There are no physical devices are available at all
/// @exception std::runtime_error No suitable physical device could be determined
/// @return The chosen physical device which is most suitable
[[nodiscard]] VkPhysicalDevice pick_best_physical_device(std::vector<DeviceInfo> &&physical_device_infos,
                                                         const VkPhysicalDeviceFeatures &required_features,
                                                         const std::span<const char *> required_extensions);

/// Pick the best physical device automatically
/// @param inst The Vulkan instance
/// @param surface The window surface
/// @param required_features The required device features
/// @param required_extensions The required device extensions
/// @return The chosen physical device which is most suitable
[[nodiscard]] VkPhysicalDevice pick_best_physical_device(const Instance &inst, VkSurfaceKHR surface,
                                                         const VkPhysicalDeviceFeatures &required_features,
                                                         const std::span<const char *> required_extensions);

} // namespace inexor::vulkan_renderer::tools
