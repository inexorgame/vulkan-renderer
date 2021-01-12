#pragma once

#include "inexor/vulkan-renderer/exceptions/exception.hpp"

#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer::exceptions {

/// @brief
class VulkanException final : public Exception {
public:
    /// @brief Default constructor.
    /// @param message The exception message.
    /// @param result The VkResult value of the Vulkan API call which failed.
    VulkanException(const std::string &message, VkResult result);
};

} // namespace inexor::vulkan_renderer::exceptions
