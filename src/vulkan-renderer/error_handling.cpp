#include "inexor/vulkan-renderer/error_handling.hpp"

#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace inexor::vulkan_renderer {

/// @brief Returns a user friendly error description text.
/// @param result_code The error code.
std::string get_error_description_text(const VkResult &result_code) {
    std::string error_string = "";

    // For more information on error codes see:
    // https://github.com/SaschaWillems/Vulkan and
    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkResult.html

    switch (result_code) {
    case VK_SUCCESS:
        error_string = "Command successfully completed.";
        break;
    case VK_NOT_READY:
        error_string = "A fence or query has not yet completed.";
        break;
    case VK_TIMEOUT:
        error_string = "A wait operation has not completed in the specified time.";
        break;
    case VK_EVENT_SET:
        error_string = "An event is signaled.";
        break;
    case VK_EVENT_RESET:
        error_string = "An event is unsignaled.";
        break;
    case VK_INCOMPLETE:
        error_string = "A return array was too small for the result.";
        break;
    case VK_SUBOPTIMAL_KHR:
        error_string = "A swapchain no longer matches the surface properties exactly, but can still be used to present "
                       "to the surface successfully.";
        break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        error_string = "A host memory allocation has failed.";
        break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        error_string = "A device memory allocation has failed.";
        break;
    case VK_ERROR_INITIALIZATION_FAILED:
        error_string = "Initialization of an object could not be completed for implementation-specific reasons.";
        break;
    case VK_ERROR_DEVICE_LOST:
        error_string = "The logical or physical device has been lost. See Lost Device.";
        break;
    case VK_ERROR_MEMORY_MAP_FAILED:
        error_string = "Mapping of a memory object has failed.";
        break;
    case VK_ERROR_LAYER_NOT_PRESENT:
        error_string = "A requested layer is not present or could not be loaded.";
        break;
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        error_string = "A requested extension is not supported.";
        break;
    case VK_ERROR_FEATURE_NOT_PRESENT:
        error_string = "A requested feature is not supported.";
        break;
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        error_string = "The requested version of Vulkan is not supported by the driver or is otherwise incompatible "
                       "for implementation-specific reasons.";
        break;
    case VK_ERROR_TOO_MANY_OBJECTS:
        error_string = "Too many objects of the type have already been created.";
        break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        error_string = "A requested format is not supported on this device.";
        break;
    case VK_ERROR_FRAGMENTED_POOL:
        error_string =
            "A pool allocation has failed due to fragmentation of the pool's memory. This must only be returned if no "
            "attempt to allocate host or "
            "device memory was made to accommodate the new allocation. This should be returned in preference to "
            "VK_ERROR_OUT_OF_POOL_MEMORY, but "
            "only if the implementation is certain that the pool allocation failure was due to fragmentation.";
        break;
    case VK_ERROR_SURFACE_LOST_KHR:
        error_string = "A surface is no longer available.";
        break;
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        error_string = "The requested window is already in use by Vulkan or another API in a manner which prevents it "
                       "from being used again.";
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
        error_string = "A surface has changed in such a way that it is no longer compatible with the swapchain, and "
                       "further presentation requests using the swapchain "
                       "will fail. Applications must query the new surface properties and recreate their swapchain if "
                       "they wish to continue presenting to the surface.";
        break;
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        error_string = "The display used by a swapchain does not use the same presentable image layout, or is "
                       "incompatible in a way that prevents sharing an image.";
        break;
    case VK_ERROR_INVALID_SHADER_NV:
        error_string =
            "One or more shaders failed to compile or link. More details are reported back to the application via "
            "https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VK_EXT_debug_report if "
            "enabled.";
        break;
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        error_string = "A pool memory allocation has failed. This must only be returned if no attempt to allocate host "
                       "or device memory was made to accommodate the new "
                       "allocation. If the failure was definitely due to fragmentation of the pool, "
                       "VK_ERROR_FRAGMENTED_POOL should be returned instead.";
        break;
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        error_string = "An external handle is not a valid handle of the specified type.";
        break;
    case VK_ERROR_FRAGMENTATION_EXT:
        error_string = "A descriptor pool creation has failed due to fragmentation.";
        break;
    case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        error_string = "A buffer creation failed because the requested address is not available.";
        break;
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        error_string = "An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT "
                       "failed as it did not have exlusive "
                       "full-screen access. This may occur due to implementation-dependent reasons, outside of the "
                       "application's control.";
        break;
    default:
        error_string = "Unknown error";
        break;
    }

    return error_string;
}

void display_error_message(const std::string &error_message, const std::string &message_box_title) {
    // Also print error message to console.
    spdlog::error(error_message);

#ifdef _WIN32
    // Windows-specific message box.
    MessageBox(NULL, error_message.c_str(), message_box_title.c_str(), MB_OK | MB_ICONERROR);
#endif

    // TODO: Add Linux specific error message box.
    // TODO: Add Mac-OS specific error message box.
}

void display_fatal_error_message(const std::string &error_message, const std::string &message_box_title) {
    // Also print error message to console.
    spdlog::critical(error_message);

#ifdef _WIN32
    // Windows-specific message box.
    MessageBox(NULL, error_message.c_str(), message_box_title.c_str(), MB_OK | MB_ICONERROR);
#endif

    // TODO: Add Linux specific error message box.
    // TODO: Add Mac-OS specific error message box.
}

void display_warning_message(const std::string &warning_message, const std::string &message_box_title) {
    // Also print error message to console.
    spdlog::warn(warning_message);

#ifdef _WIN32
    // Windows-specific message box.
    MessageBox(NULL, warning_message.c_str(), message_box_title.c_str(), MB_OK | MB_ICONWARNING);
#endif

    // TODO: Add Linux specific error message box.
    // TODO: Add Mac-OS specific error message box.
}

void vulkan_error_check(const VkResult &result) {
    if (result != VK_SUCCESS) {
        std::string error_message = "Error: " + get_error_description_text(result);
        display_error_message(error_message);
    }
}

} // namespace inexor::vulkan_renderer
