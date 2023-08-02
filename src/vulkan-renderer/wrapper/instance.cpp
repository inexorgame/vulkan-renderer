#include "inexor/vulkan-renderer/wrapper/instance.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/vk_tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <GLFW/glfw3.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

bool Instance::is_layer_supported(const std::string &layer_name) {
    std::uint32_t instance_layer_count = 0;

    if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr); result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateInstanceLayerProperties failed!", result);
    }

    if (instance_layer_count == 0) {
        // This is not an error. Some platforms simply don't have any instance layers.
        spdlog::info("No Vulkan instance layers available!");
        return false;
    }

    std::vector<VkLayerProperties> instance_layers(instance_layer_count);

    // Store all available instance layers.
    if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateInstanceLayerProperties failed!", result);
    }

    // Search for the requested instance layer.
    return std::find_if(instance_layers.begin(), instance_layers.end(), [&](const VkLayerProperties instance_layer) {
               return instance_layer.layerName == layer_name;
           }) != instance_layers.end();
}

bool Instance::is_extension_supported(const std::string &extension_name) {
    std::uint32_t instance_extension_count = 0;

    if (const auto result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateInstanceExtensionProperties failed!", result);
    }

    if (instance_extension_count == 0) {
        // This is not an error. Some platforms simply don't have any instance extensions.
        spdlog::info("No Vulkan instance extensions available!");
        return false;
    }

    std::vector<VkExtensionProperties> instance_extensions(instance_extension_count);

    // Store all available instance extensions.
    if (const auto result =
            vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateInstanceExtensionProperties failed!", result);
    }

    // Search for the requested instance extension.
    return std::find_if(instance_extensions.begin(), instance_extensions.end(),
                        [&](const VkExtensionProperties instance_extension) {
                            return instance_extension.extensionName == extension_name;
                        }) != instance_extensions.end();
}

Instance::Instance(const std::string &application_name, const std::string &engine_name,
                   const std::uint32_t application_version, const std::uint32_t engine_version,
                   bool enable_validation_layers, const PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
                   const std::vector<std::string> &requested_instance_extensions,
                   const std::vector<std::string> &requested_instance_layers) {
    if (application_name.empty()) {
        throw std::invalid_argument("Error: Application name is empty!");
    }
    if (engine_name.empty()) {
        throw std::invalid_argument("Error: Engine name is empty!");
    }
    if (debug_callback == nullptr) {
        throw std::invalid_argument("Error: Invalid debug utils messenger callback!");
    }

    spdlog::trace("Initializing Vulkan metaloader");
    if (const auto result = volkInitialize(); result != VK_SUCCESS) {
        throw std::runtime_error("Error: Could not initialize Vulkan with volk metaloader! Make sure to update the "
                                 "drivers of your graphics card!");
    }

    spdlog::trace("Initialising Vulkan instance");
    spdlog::trace("Application name: {}", application_name);
    spdlog::trace("Application version: {}.{}.{}", VK_API_VERSION_MAJOR(application_version),
                  VK_API_VERSION_MINOR(application_version), VK_API_VERSION_PATCH(application_version));
    spdlog::trace("Engine name: {}", engine_name);
    spdlog::trace("Engine version: {}.{}.{}", VK_API_VERSION_MAJOR(engine_version),
                  VK_API_VERSION_MINOR(engine_version), VK_API_VERSION_PATCH(engine_version));
    spdlog::trace("Requested Vulkan API version: {}.{}.{}", VK_API_VERSION_MAJOR(REQUIRED_VK_API_VERSION),
                  VK_API_VERSION_MINOR(REQUIRED_VK_API_VERSION), VK_API_VERSION_PATCH(REQUIRED_VK_API_VERSION));

    std::uint32_t available_api_version = 0;
    if (const auto result = vkEnumerateInstanceVersion(&available_api_version); result != VK_SUCCESS) {
        spdlog::error("Error: vkEnumerateInstanceVersion returned {}!", vk_tools::as_string(result));
        return;
    }

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
        throw std::runtime_error(exception_message);
    }

    const auto app_info = make_info<VkApplicationInfo>({
        .pApplicationName = application_name.c_str(),
        .applicationVersion = application_version,
        .pEngineName = engine_name.c_str(),
        .engineVersion = engine_version,
        .apiVersion = REQUIRED_VK_API_VERSION,
    });

    std::vector<const char *> instance_extension_wishlist = {
#ifndef NDEBUG
        // VK_EXT_debug_utils
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    };

    std::uint32_t glfw_extension_count = 0;

    // Because this requires some dynamic libraries to be loaded, this may take even up to some seconds!
    auto *glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    if (glfw_extension_count == 0) {
        throw std::runtime_error(
            "Error: glfwGetRequiredInstanceExtensions results 0 as number of required instance extensions!");
    }

    spdlog::trace("Required GLFW instance extensions:");

    // Add all instance extensions which are required by GLFW to our wishlist.
    for (std::size_t i = 0; i < glfw_extension_count; i++) {
        spdlog::trace("   - {}", glfw_extensions[i]);              // NOLINT
        instance_extension_wishlist.push_back(glfw_extensions[i]); // NOLINT
    }

    // We have to check which instance extensions of our wishlist are available on the current system!
    // Add requested instance extensions to wishlist.
    for (const auto &requested_instance_extension : requested_instance_extensions) {
        instance_extension_wishlist.push_back(requested_instance_extension.c_str());
    }

    std::vector<const char *> enabled_instance_extensions{};

    spdlog::trace("List of enabled instance extensions:");

    // We are not checking for duplicated entries but this is no problem.
    for (const auto &instance_extension : instance_extension_wishlist) {
        if (is_extension_supported(instance_extension)) {
            spdlog::trace("   - {} ", instance_extension);
            enabled_instance_extensions.push_back(instance_extension);
        } else {
            spdlog::error("Requested instance extension {} is not available on this system!", instance_extension);
        }
    }

    std::vector<const char *> instance_layers_wishlist{};

    spdlog::trace("Instance layer wishlist:");

#ifndef NDEBUG
    // We can't stress enough how important it is to use validation layers during development!
    // Validation layers in Vulkan are in-depth error checks for the application's use of the API.
    // They check for a multitude of possible errors. They can be disabled easily for releases.
    // Understand that in contrary to other APIs, in Vulkan API the driver provides no error checks
    // for you! If you use Vulkan API incorrectly, your application will likely just crash.
    // To avoid this, you must use validation layers during development!
    if (enable_validation_layers) {
        spdlog::trace("   - VK_LAYER_KHRONOS_validation");
        instance_layers_wishlist.push_back("VK_LAYER_KHRONOS_validation");
    }

#endif

    // Add requested instance layers to wishlist.
    for (const auto &instance_layer : requested_instance_layers) {
        instance_layers_wishlist.push_back(instance_layer.c_str());
    }

    std::vector<const char *> enabled_instance_layers{};

    spdlog::trace("List of enabled instance layers:");

    // We have to check which instance layers of our wishlist are available on the current system!
    // We are not checking for duplicated entries but this is no problem.
    for (const auto &current_layer : instance_layers_wishlist) {
        if (is_layer_supported(current_layer)) {
            spdlog::trace("   - {}", current_layer);
            enabled_instance_layers.push_back(current_layer);
        } else {
            spdlog::warn("Requested instance layer {} is not available on this system!", current_layer);
        }
    }

    const auto instance_ci = make_info<VkInstanceCreateInfo>({
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<std::uint32_t>(enabled_instance_layers.size()),
        .ppEnabledLayerNames = enabled_instance_layers.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(enabled_instance_extensions.size()),
        .ppEnabledExtensionNames = enabled_instance_extensions.data(),
    });

    if (const auto result = vkCreateInstance(&instance_ci, nullptr, &m_instance); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateInstance failed!", result);
    }

    volkLoadInstanceOnly(m_instance);

    // Note that we can only call is_extension_supported afer volkLoadInstanceOnly!
    if (!is_extension_supported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        // Don't forget to destroy the instance before throwing the exception!
        vkDestroyInstance(m_instance, nullptr);
        throw std::runtime_error("Error: VK_EXT_DEBUG_UTILS_EXTENSION_NAME is not supported!");
    }

    const auto dbg_messenger_ci = make_info<VkDebugUtilsMessengerCreateInfoEXT>({
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback,
        .pUserData = nullptr,
    });

    if (const auto result = vkCreateDebugUtilsMessengerEXT(m_instance, &dbg_messenger_ci, nullptr, &m_debug_callback);
        result != VK_SUCCESS) {
        // Don't forget to destroy the instance before throwing the exception!
        vkDestroyInstance(m_instance, nullptr);
        throw VulkanException(
            "Error: Could not create Vulkan validation layer debug callback! (vkCreateDebugUtilsMessengerEXT failed!)",
            result);
    }
}

Instance::Instance(Instance &&other) noexcept {
    m_instance = std::exchange(other.m_instance, nullptr);
    m_debug_callback = std::exchange(other.m_debug_callback, nullptr);
}

Instance::~Instance() {
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_callback, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
