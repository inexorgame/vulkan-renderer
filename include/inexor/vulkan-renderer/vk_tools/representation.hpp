#pragma once

#include <volk.h>

#include <string>

namespace inexor::vulkan_renderer::vk_tools {

/// @brief This function returns a textual representation of the vulkan object T.
template <typename VulkanObjectType>
[[nodiscard]] std::string_view as_string(VulkanObjectType);

/// Get a feature description of a ``VkBool32`` value in the ``VkPhysicalDeviceFeatures`` struct by index.
/// @param index The index of the ``VkBool32`` value in the ``VkPhysicalDeviceFeatures`` struct.
/// @note If the index is out of bounds, no exception will be thrown, but an empty description will be returned instead.
/// @return A feature description
[[nodiscard]] std::string_view get_device_feature_description(std::size_t index);

/// This template allows us to convert a template parameter name into a VkObjectType
/// @note We have to specify a specialization for every Vulkan object type!
/// As far as we know, there is no other easy way to do this in C++.
/// @tparam VulkanObjectType The Vulkan object type as template parameter, for examplke VkFence
/// @return The VkObjectType of the template parameter, for the above mentioned example ``VK_OBJECT_TYPE_FENCE``
template <typename VulkanObjectType>
[[nodiscard]] constexpr VkObjectType get_vulkan_object_type(VulkanObjectType);

/// @brief Convert a VkResult value into the corresponding error description as std::string_view
/// @param result The VkResult to convert
/// @return A std::string_view which contains an error description text of the VkResult
/// @note This function converts the VkResult into the corresponding error description text
/// If you want to convert it into an std::string_view, see the matching ```as_string``` template
[[nodiscard]] std::string_view result_to_description(VkResult result);

} // namespace inexor::vulkan_renderer::vk_tools
