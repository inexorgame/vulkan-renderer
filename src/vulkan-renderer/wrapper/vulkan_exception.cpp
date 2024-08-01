#include "inexor/vulkan-renderer/wrapper/vulkan_exception.hpp"

#include "inexor/vulkan-renderer/tools/representation.hpp"

namespace inexor::vulkan_renderer::wrapper {

// TODO (GH-468): Use std::source_location in exceptions
VulkanException::VulkanException(std::string message, const VkResult result)
    : InexorException(message.append(" (")
                          .append(tools::as_string(result))
                          .append(": ")
                          .append(tools::result_to_description(result))
                          .append(")")) {}

} // namespace inexor::vulkan_renderer::wrapper
