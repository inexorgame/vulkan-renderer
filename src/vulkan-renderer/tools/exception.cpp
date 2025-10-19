#include "inexor/vulkan-renderer/tools/exception.hpp"

#include "inexor/vulkan-renderer/tools/representation.hpp"

namespace inexor::vulkan_renderer::tools {

InexorException::InexorException(std::string message, std::source_location location)
    : std::runtime_error("\n\tFILE:\t  " + std::string(location.file_name()) +
                         "\n\tFUNCTION: " + std::string(location.function_name()) + "\n\tLINE:\t  " +
                         std::to_string(location.line()) + "\n\tERROR:\t  " + std::string(message)) {}

VulkanException::VulkanException(std::string message, const VkResult result, const std::string object_name,
                                 const std::source_location location)
    : InexorException(message.append("[")
                          .append(object_name)
                          .append("]")
                          .append(" (")
                          .append(tools::as_string(result))
                          .append(": ")
                          .append(tools::result_to_description(result))
                          .append(")"),
                      location) {}

} // namespace inexor::vulkan_renderer::tools
