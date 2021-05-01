#include "inexor/vulkan-renderer/exception.hpp"

#include "inexor/vulkan-renderer/vk_tools/representation.hpp"

namespace inexor::vulkan_renderer {

VulkanException::VulkanException(std::string message, const VkResult result)
    : InexorException(message.append(std::string_view(" ("))
                          .append(vk_tools::result_to_string(result))
                          .append(std::string_view(": "))
                          .append(vk_tools::result_to_description(result))
                          .append(std::string_view(")"))) {}

} // namespace inexor::vulkan_renderer
