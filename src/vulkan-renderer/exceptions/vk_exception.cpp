#include "inexor/vulkan-renderer/exceptions/vk_exception.hpp"

#include "inexor/vulkan-renderer/vk_tools/representation.hpp"

namespace inexor::vulkan_renderer::exceptions {

VulkanException::VulkanException(const std::string &message, const VkResult result)
    : Exception(message + " (" + vk_tools::result_to_string(result) + ": " + vk_tools::result_to_description(result) +
                ")") {}

} // namespace inexor::vulkan_renderer::exceptions
