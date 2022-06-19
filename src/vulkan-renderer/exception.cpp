#include "inexor/vulkan-renderer/exception.hpp"

#include "inexor/vulkan-renderer/vk_tools/representation.hpp"

namespace inexor::vulkan_renderer {

VulkanException::VulkanException(std::string message, const VkResult result, const std::source_location location)
    : InexorException(message.append(" (")
                          .append(vk_tools::as_string(result))
                          .append(": ")
                          .append(vk_tools::result_to_description(result))
                          .append(") (")
                          .append("file: ")
                          .append(location.file_name())
                          .append(", line: ")
                          .append(std::to_string(location.line()))
                          .append(", column: ")
                          .append(std::to_string(location.column()))
                          .append(", function name: ")
                          .append(location.function_name())
                          .append(")")) {}

} // namespace inexor::vulkan_renderer
