#include "inexor/vulkan-renderer/wrapper/instance.hpp"

#include "inexor/vulkan-renderer/meta/meta.hpp"
#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

// Using declarations
using tools::InexorException;
using tools::VulkanException;

bool is_instance_extension_supported(const std::string &extension_name) {
    static const auto &instance_extensions = tools::get_instance_extensions();
    static const auto &instance_layers = tools::get_instance_layers();

    // Let's check if this instance extension is really an instance extension and not an instance layer by accident!
    // You would not believe how often this mistake happened to us, so let's add a safety check here!
    if (std::find_if(instance_layers.begin(), instance_layers.end(), [&](const VkLayerProperties &instance_layer) {
            return std::strcmp(instance_layer.layerName, extension_name.c_str()) == 0;
        }) != instance_layers.end()) {
        throw InexorException("Error: '" + extension_name + "' is an instance layer, not an instance extension!");
    }
    // Search for the requested instance extension.
    return std::find_if(instance_extensions.begin(), instance_extensions.end(),
                        [&](const VkExtensionProperties &instance_extension) {
                            return std::strcmp(instance_extension.extensionName, extension_name.c_str()) == 0;
                        }) != instance_extensions.end();
}

bool is_instance_layer_supported(const std::string &layer_name) {
    static const auto &instance_layers = tools::get_instance_layers();
    static const auto &instance_extensions = tools::get_instance_extensions();

    // Let's check if this instance layer is really an instance layer and not an instance extension by accident.
    // You would not believe how often this mistake happened to us, so let's add a safety check here!
    if (std::find_if(instance_extensions.begin(), instance_extensions.end(),
                     [&](const VkExtensionProperties &instance_extension) {
                         return std::strcmp(instance_extension.extensionName, layer_name.c_str()) == 0;
                     }) != instance_extensions.end()) {
        throw InexorException("Error: '" + layer_name + "' is an instance extension, not an instance layer!");
    }
    // Search for the requested instance layer.
    return std::find_if(instance_layers.begin(), instance_layers.end(), [&](const VkLayerProperties &instance_layer) {
               return std::strcmp(instance_layer.layerName, layer_name.c_str()) == 0;
           }) != instance_layers.end();
}

Instance::Instance(const std::span<const char *> instance_layers, const std::span<const char *> instance_extensions) {
    spdlog::trace("Application name: {}", meta::APP_VERSION_STR);
    spdlog::trace("Application version: {}", meta::APP_VERSION_STR);
    spdlog::trace("Engine name: {}", meta::ENGINE_NAME);
    spdlog::trace("Engine version: {}", meta::ENGINE_VERSION_STR);
    spdlog::trace("Requested Vulkan API version: {}.{}.{}", VK_API_VERSION_MAJOR(REQUIRED_VK_API_VERSION),
                  VK_API_VERSION_MINOR(REQUIRED_VK_API_VERSION), VK_API_VERSION_PATCH(REQUIRED_VK_API_VERSION));

    // Let's add these extra checks to make sure the programmer did not forget to call volkInitialize();
    // We cannot put volkInitialize() into this constructor, because at the time of calling the constructor,
    // the programmer must already have checked if the instance layers and instance extensions which are passed to this
    // constructor are available on the system or not. This implies that the oder of initialization is volkInitialize(),
    // then checking if all required instance layers and instance extensions are available on the system with
    // `is_instance_extension_supported` and `is_instance_layer_supported`, then creating an instance of this class!
    if (vkEnumerateInstanceVersion == nullptr) {
        throw InexorException("Error: Function pointer 'vkEnumerateInstanceVersion' is not available!");
    }
    if (vkCreateInstance == nullptr) {
        throw InexorException("Error: Function pointer 'vkCreateInstance' is not available!");
    }

    std::uint32_t available_api_version = 0;
    if (const auto result = vkEnumerateInstanceVersion(&available_api_version); result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateInstanceVersion failed!", result);
    }

    spdlog::trace("Available Vulkan API version: {}.{}.{}", VK_API_VERSION_MAJOR(available_api_version),
                  VK_API_VERSION_MINOR(available_api_version), VK_API_VERSION_PATCH(available_api_version));

    // This code will throw an exception if the required version of Vulkan API is not available on the system
    if (VK_API_VERSION_MAJOR(REQUIRED_VK_API_VERSION) > VK_API_VERSION_MAJOR(available_api_version) ||
        (VK_API_VERSION_MAJOR(REQUIRED_VK_API_VERSION) == VK_API_VERSION_MAJOR(available_api_version) &&
         VK_API_VERSION_MINOR(REQUIRED_VK_API_VERSION) > VK_API_VERSION_MINOR(available_api_version))) {
        std::string exception_message = fmt::format(
            "Your system does not support the required version of Vulkan API. Required version: {}.{}.{}. Available "
            "Vulkan API version on this machine: {}.{}.{}. Please update your graphics drivers!",
            std::to_string(VK_API_VERSION_MAJOR(REQUIRED_VK_API_VERSION)),
            std::to_string(VK_API_VERSION_MINOR(REQUIRED_VK_API_VERSION)),
            std::to_string(VK_API_VERSION_PATCH(REQUIRED_VK_API_VERSION)),
            std::to_string(VK_API_VERSION_MAJOR(available_api_version)),
            std::to_string(VK_API_VERSION_MINOR(available_api_version)),
            std::to_string(VK_API_VERSION_PATCH(available_api_version)));
        throw InexorException(exception_message);
    }

    const auto app_info = make_info<VkApplicationInfo>({
        .pApplicationName = meta::APP_NAME,
        .applicationVersion = meta::APP_VERSION_UI32,
        .pEngineName = meta::ENGINE_NAME,
        .engineVersion = meta::ENGINE_VERSION_UI32,
        .apiVersion = REQUIRED_VK_API_VERSION,
    });

    const auto instance_ci = make_info<VkInstanceCreateInfo>({
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<std::uint32_t>(instance_layers.size()),
        .ppEnabledLayerNames = instance_layers.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(instance_extensions.size()),
        .ppEnabledExtensionNames = instance_extensions.data(),
    });

    spdlog::trace("Initialising Vulkan instance");
    if (const auto result = vkCreateInstance(&instance_ci, nullptr, &m_instance); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateInstance failed!", result);
    }

    spdlog::trace("Loading Vulkan instance-level function pointers with volkLoadInstanceOnly");
    volkLoadInstanceOnly(m_instance);

    // vkDestroyInstance is loaded by volkLoadInstanceOnly.
    if (vkDestroyInstance == nullptr) {
        // This should practically be impossible, but let's just be sure.
        throw InexorException("Error: Function pointer 'vkDestroyInstance' is not available!");
    }
}

Instance::Instance(Instance &&other) noexcept {
    m_instance = std::exchange(other.m_instance, nullptr);
}

Instance::~Instance() {
    vkDestroyInstance(m_instance, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
