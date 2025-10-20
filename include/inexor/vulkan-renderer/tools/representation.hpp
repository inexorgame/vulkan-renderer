#pragma once

#include <volk.h>

#include <string>
#include <type_traits>

namespace inexor::vulkan_renderer::tools {

/// @brief This function returns a textual representation of the vulkan object T.
template <typename T>
[[nodiscard]] std::string_view as_string(T);

/// Get a feature description of a ``VkBool32`` value in the ``VkPhysicalDeviceFeatures`` struct by index.
/// @param index The index of the ``VkBool32`` value in the ``VkPhysicalDeviceFeatures`` struct.
/// @note If the index is out of bounds, no exception will be thrown, but an empty description will be returned instead.
/// @return A feature description
[[nodiscard]] std::string_view get_device_feature_description(std::size_t index);

/// A template which turns a Vulkan object type into a VK_OBJECT_TYPE
/// @tparam T The Vulkan object type
/// @return The associated
template <typename T>
[[nodiscard]] constexpr VkObjectType get_vulkan_object_type(const T &) noexcept {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;

    if constexpr (std::is_same_v<U, VkBuffer>)
        return VK_OBJECT_TYPE_BUFFER;
    else if constexpr (std::is_same_v<U, VkCommandBuffer>)
        return VK_OBJECT_TYPE_COMMAND_BUFFER;
    else if constexpr (std::is_same_v<U, VkCommandPool>)
        return VK_OBJECT_TYPE_COMMAND_POOL;
    else if constexpr (std::is_same_v<U, VkInstance>)
        return VK_OBJECT_TYPE_INSTANCE;
    else if constexpr (std::is_same_v<U, VkPhysicalDevice>)
        return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
    else if constexpr (std::is_same_v<U, VkDescriptorPool>)
        return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
    else if constexpr (std::is_same_v<U, VkDescriptorSet>)
        return VK_OBJECT_TYPE_DESCRIPTOR_SET;
    else if constexpr (std::is_same_v<U, VkDescriptorSetLayout>)
        return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
    else if constexpr (std::is_same_v<U, VkDevice>)
        return VK_OBJECT_TYPE_DEVICE;
    else if constexpr (std::is_same_v<U, VkEvent>)
        return VK_OBJECT_TYPE_EVENT;
    else if constexpr (std::is_same_v<U, VkFence>)
        return VK_OBJECT_TYPE_FENCE;
    // TODO: Remove when migrating to dynamic rendering!
    else if constexpr (std::is_same_v<U, VkFramebuffer>)
        return VK_OBJECT_TYPE_FRAMEBUFFER;
    else if constexpr (std::is_same_v<U, VkImage>)
        return VK_OBJECT_TYPE_IMAGE;
    else if constexpr (std::is_same_v<U, VkImageView>)
        return VK_OBJECT_TYPE_IMAGE_VIEW;
    else if constexpr (std::is_same_v<U, VkPipeline>)
        return VK_OBJECT_TYPE_PIPELINE;
    else if constexpr (std::is_same_v<U, VkPipelineCache>)
        return VK_OBJECT_TYPE_PIPELINE_CACHE;
    else if constexpr (std::is_same_v<U, VkPipelineLayout>)
        return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    else if constexpr (std::is_same_v<U, VkQueryPool>)
        return VK_OBJECT_TYPE_QUERY_POOL;
    else if constexpr (std::is_same_v<U, VkQueue>)
        return VK_OBJECT_TYPE_QUEUE;
    // TODO: Remove when migrating to dynamic rendering!
    else if constexpr (std::is_same_v<U, VkRenderPass>)
        return VK_OBJECT_TYPE_RENDER_PASS;
    else if constexpr (std::is_same_v<U, VkSampler>)
        return VK_OBJECT_TYPE_SAMPLER;
    else if constexpr (std::is_same_v<U, VkSemaphore>)
        return VK_OBJECT_TYPE_SEMAPHORE;
    else if constexpr (std::is_same_v<U, VkShaderModule>)
        return VK_OBJECT_TYPE_SHADER_MODULE;
    else if constexpr (std::is_same_v<U, VkSurfaceKHR>)
        return VK_OBJECT_TYPE_SURFACE_KHR;
    else if constexpr (std::is_same_v<U, VkSwapchainKHR>)
        return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
    else {
        return VK_OBJECT_TYPE_UNKNOWN;
    }
}

/// @brief Convert a VkResult value into the corresponding error description as std::string_view
/// @param result The VkResult to convert
/// @return A std::string_view which contains an error description text of the VkResult
/// @note This function converts the VkResult into the corresponding error description text
/// If you want to convert it into an std::string_view, see the matching ```as_string``` template
[[nodiscard]] std::string_view result_to_description(VkResult result);

} // namespace inexor::vulkan_renderer::tools
