#pragma once

#include <volk.h>

#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer {

/// A custom base class for exceptions
class InexorException : public std::runtime_error {
public:
    // No need to define own constructors.
    using std::runtime_error::runtime_error;
};

/// InexorException for Vulkan specific things.
class VulkanException final : public InexorException {
public:
    /// Default constructor
    /// @param message The exception message.
    /// @param result The VkResult value of the Vulkan API call which failed.
    VulkanException(std::string message, VkResult result);
};

} // namespace inexor::vulkan_renderer
