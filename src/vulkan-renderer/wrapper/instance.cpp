#include "inexor/vulkan-renderer/wrapper/instance.hpp"

#include "inexor/vulkan-renderer/exceptions/vk_exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <GLFW/glfw3.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

Instance::Instance(const std::string &application_name, const std::string &engine_name,
                   const std::uint32_t application_version, const std::uint32_t engine_version,
                   const std::uint32_t vulkan_api_version, bool enable_validation_layers,
                   bool enable_renderdoc_instance_layer, std::vector<std::string> requested_instance_extensions,
                   std::vector<std::string> requested_instance_layers) {
    assert(!application_name.empty());
    assert(!engine_name.empty());

    // In Vulkan API we can use VK_MAKE_VERSION() macro to create an std::uint32_t value from major, minor and patch
    // number.

    spdlog::debug("Initialising Vulkan instance.");
    spdlog::debug("Application name: '{}'", application_name);
    spdlog::debug("Application version: {}.{}.{}", VK_VERSION_MAJOR(application_version),
                  VK_VERSION_MINOR(application_version), VK_VERSION_PATCH(application_version));
    spdlog::debug("Engine name: '{}'", engine_name);
    spdlog::debug("Engine version: {}.{}.{}", VK_VERSION_MAJOR(engine_version), VK_VERSION_MINOR(engine_version),
                  VK_VERSION_PATCH(engine_version));
    spdlog::debug("Requested Vulkan API version: {}.{}.{}", VK_VERSION_MAJOR(vulkan_api_version),
                  VK_VERSION_MINOR(vulkan_api_version), VK_VERSION_PATCH(vulkan_api_version));

    auto app_info = make_info<VkApplicationInfo>();
    app_info.pApplicationName = application_name.c_str();
    app_info.applicationVersion = application_version;
    app_info.pEngineName = engine_name.c_str();
    app_info.engineVersion = engine_version;
    app_info.apiVersion = vulkan_api_version;

    std::vector<const char *> instance_extension_wishlist = {
#ifndef NDEBUG
        // In debug mode, we use the following instance extensions:
        // This one is for assigning internal names to Vulkan resources.
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        // This one is for setting up a Vulkan debug report callback function.
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
    };

    std::uint32_t glfw_extension_count = 0;

    // Because this requires some dynamic libraries to be loaded, this may take even up to some seconds!
    auto *glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    if (glfw_extension_count == 0) {
        throw std::runtime_error(
            "Error: glfwGetRequiredInstanceExtensions results 0 as number of required instance extensions!");
    }

    spdlog::debug("Required GLFW instance extensions:");

    // Add all instance extensions which are required by GLFW to our wishlist.
    for (std::size_t i = 0; i < glfw_extension_count; i++) {
        spdlog::debug(glfw_extensions[i]);
        instance_extension_wishlist.push_back(glfw_extensions[i]);
    }

    // We have to check which instance extensions of our wishlist are available on the current system!
    // Add requested instance extensions to wishlist.
    for (const auto &requested_instance_extension : requested_instance_extensions) {
        instance_extension_wishlist.push_back(requested_instance_extension.c_str());
    }

    std::vector<const char *> enabled_instance_extensions{};

    // We are not checking for duplicated entries but this is no problem.
    for (const auto &instance_extension : instance_extension_wishlist) {
        if (m_availability_checks.has_instance_extension(instance_extension)) {
            spdlog::debug("Adding '{}' to list of enabled instance extensions.", instance_extension);
            enabled_instance_extensions.push_back(instance_extension);
        } else {
            spdlog::error("Requested instance extension '{}' is not available on this system!", instance_extension);
        }
    }

    std::vector<const char *> instance_layers_wishlist{};

#ifndef NDEBUG
    // RenderDoc is a very useful open source graphics debugger for Vulkan and other APIs.
    // Not using it all the time during development is fine, but as soon as something crashes
    // you should enable it, take a snapshot and look up what's wrong.
    if (enable_renderdoc_instance_layer) {
        instance_layers_wishlist.push_back("VK_LAYER_RENDERDOC_Capture");
    }

    // We can't stress enough how important it is to use validation layers during development!
    // Validation layers in Vulkan are in-depth error checks for the application's use of the API.
    // They check for a multitude of possible errors. They can be disabled easily for releases.
    // Understand that in contrary to other APIs, in Vulkan API the driver provides no error checks
    // for you! If you use Vulkan API incorrectly, your application will likely just crash.
    // To avoid this, you must use validation layers during development!
    if (enable_validation_layers) {
        spdlog::debug("Vulkan validation layers are enabled.");
        spdlog::debug("Adding 'VK_LAYER_KHRONOS_validation' to instance extension wishlist.");
        instance_layers_wishlist.push_back("VK_LAYER_KHRONOS_validation");
    } else {
        spdlog::warn("Vulkan validation layers are not enabled although debug configuration is selected!");
        spdlog::warn("You must always use validation layers during development if you are serious about writing stable "
                     "software.");
        spdlog::warn("Without them, most bugs are literally impossible to find.");
        spdlog::warn("Even worse, your software might work on your machine but does crash on other systems!");
    }
#endif

    // Add requested instance layers to wishlist.
    for (const auto &instance_layer : requested_instance_layers) {
        instance_layers_wishlist.push_back(instance_layer.c_str());
    }

    std::vector<const char *> enabled_instance_layers{};

    // We have to check which instance layers of our wishlist are available on the current system!
    // We are not checking for duplicated entries but this is no problem.
    for (const auto &current_layer : instance_layers_wishlist) {
        if (m_availability_checks.has_instance_layer(current_layer)) {
            spdlog::debug("Adding '{}' to list of enabled instance layers.", current_layer);
            enabled_instance_layers.push_back(current_layer);
        } else {
#ifdef NDEBUG
            if (std::string(current_layer) == VK_EXT_DEBUG_MARKER_EXTENSION_NAME) {
                spdlog::error("You can't use command line argument -renderdoc in release mode.");
            }
#else
            spdlog::error("Requested instance layer '{}' is not available on this system!", current_layer);
#endif
        }
    }

    auto instance_ci = make_info<VkInstanceCreateInfo>();
    instance_ci.pApplicationInfo = &app_info;
    instance_ci.ppEnabledExtensionNames = enabled_instance_extensions.data();
    instance_ci.enabledExtensionCount = static_cast<std::uint32_t>(enabled_instance_extensions.size());
    instance_ci.ppEnabledLayerNames = enabled_instance_layers.data();
    instance_ci.enabledLayerCount = static_cast<std::uint32_t>(enabled_instance_layers.size());

    if (const auto result = vkCreateInstance(&instance_ci, nullptr, &m_instance); result != VK_SUCCESS) {
        throw exceptions::VulkanException("Error: vkCreateInstance failed!", result);
    }

    spdlog::debug("Created Vulkan instance successfully.");
}

Instance::Instance(const std::string &application_name, const std::string &engine_name,
                   const std::uint32_t application_version, const std::uint32_t engine_version,
                   const std::uint32_t vulkan_api_version, bool enable_validation_layers, bool enable_renderdoc_layer)
    : Instance(application_name, engine_name, application_version, engine_version, vulkan_api_version,
               enable_validation_layers, enable_renderdoc_layer, {}, {}) {
    spdlog::debug("No instance extensions or instance layers specified.");
    spdlog::debug("Validation layers are requested. RenderDoc instance layer is not requested.");
}

Instance::Instance(Instance &&other) noexcept : m_instance(std::exchange(other.m_instance, nullptr)) {}

Instance::~Instance() {
    if (m_instance != nullptr) {
        spdlog::trace("Destroying instance");
        vkDestroyInstance(m_instance, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
