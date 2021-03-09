#include "inexor/vulkan-renderer/exception.hpp"

#include "inexor/vulkan-renderer/vk_tools/representation.hpp"

namespace inexor::vulkan_renderer {

VulkanException::VulkanException(const std::string &message, const VkResult result)
    : InexorException(message + " (" + vk_tools::result_to_string(result) + ": " +
                      vk_tools::result_to_description(result) + ")") {}

} // namespace inexor::vulkan_renderer
