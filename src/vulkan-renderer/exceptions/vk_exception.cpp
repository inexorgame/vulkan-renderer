#include "inexor/vulkan-renderer/exceptions/vk_exception.hpp"

namespace inexor::vulkan_renderer::exceptions {

std::string VulkanException::get_vkresult_string(const VkResult result) {
    switch (result) {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_NOT_READY:
        return "VK_NOT_READY";
    case VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_INVALID_SHADER_NV:
        return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:
        return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_ERROR_UNKNOWN:
        return "VK_ERROR_UNKNOWN";
    }

    // Don't forget to offer a default value.
    return "Unknown";
}

std::string VulkanException::get_vkresult_description(const VkResult result) {
    switch (result) {
    case VK_SUCCESS:
        return "Command successfully completed";
    case VK_NOT_READY:
        return "A fence or query has not yet completed";
    case VK_TIMEOUT:
        return "A wait operation has not completed in the specified time";
    case VK_EVENT_SET:
        return "An event is signaled";
    case VK_EVENT_RESET:
        return "An event is unsignaled";
    case VK_INCOMPLETE:
        return "A return array was too small for the result";
    case VK_SUBOPTIMAL_KHR:
        return "A swapchain no longer matches the surface properties exactly, but can still be used to present to the "
               "surface successfully.";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "A host memory allocation has failed.";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "A device memory allocation has failed.";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "Initialization of an object could not be completed for implementation-specific reasons.";
    case VK_ERROR_DEVICE_LOST:
        return "The logical or physical device has been lost. See Lost Device";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "Mapping of a memory object has failed.";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "A requested layer is not present or could not be loaded.";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "A requested extension is not supported.";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "A requested feature is not supported.";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for "
               "implementation-specific reasons.";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "Too many objects of the type have already been created.";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "A requested format is not supported on this device.";
    case VK_ERROR_FRAGMENTED_POOL:
        return "A pool allocation has failed due to fragmentation of the pool's memory. This must only be returned if "
               "no attempt to allocate host or device memory was made to accommodate the new allocation. This should "
               "be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain "
               "that the pool allocation failure was due to fragmentation.";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "A surface is no longer available.";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "The requested window is already in use by Vulkan or another API in a manner which prevents it from "
               "being used again.";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "A surface has changed in such a way that it is no longer compatible with the swapchain, and further "
               "presentation requests using the swapchain will fail. Applications must query the new surface "
               "properties and recreate their swapchain if they wish to continue presenting to the surface.";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "The display used by a swapchain does not use the same presentable image layout, or is incompatible in "
               "a way that prevents sharing an image.";
    case VK_ERROR_INVALID_SHADER_NV:
        return "One or more shaders failed to compile or link. More details are reported back to the application via "
               "VK_EXT_debug_report if enabled.";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "A pool memory allocation has failed. This must only be returned if no attempt to allocate host or "
               "device memory was made to accommodate the new allocation. If the failure was definitely due to "
               "fragmentation of the pool, VK_ERROR_FRAGMENTED_POOL should be returned instead.";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "An external handle is not a valid handle of the specified type.";
    case VK_ERROR_FRAGMENTATION:
        return "A descriptor pool creation has failed due to fragmentation.";
    case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        return "A buffer creation failed because the requested address is not available.";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT "
               "failed as it did not have exlusive full-screen access. This may occur due to implementation-dependent "
               "reasons, outside of  the application's control.";
    case VK_ERROR_UNKNOWN:
        return "An unknown error has occurred; either the application has provided invalid input, or an implementation "
               "failure has occurred.";
    }

    // Don't forget to offer a default value.
    return "Unknown";
}

VulkanException::VulkanException(const std::string message, const VkResult result)
    : Exception(message + " (" + get_vkresult_string(result) + ": " + get_vkresult_description(result) + ")") {}

} // namespace inexor::vulkan_renderer::exceptions
