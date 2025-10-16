#include "inexor/vulkan-renderer/exception.hpp"

#include "inexor/vulkan-renderer/tools/representation.hpp"

namespace inexor::vulkan_renderer {

VulkanException::VulkanException(std::string message, const VkResult result)
    : InexorException(message.append(" (")
                          .append(tools::as_string(result))
                          .append(": ")
                          .append(tools::result_to_description(result))
                          .append(")")) {}

} // namespace inexor::vulkan_renderer
