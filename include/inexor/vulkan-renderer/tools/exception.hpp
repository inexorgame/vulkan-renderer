#pragma once

#include <volk.h>

#include <source_location>
#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer::tools {

/// A custom base class for exceptions
class InexorException : public std::runtime_error {
public:
    /// Default constructor
    /// @param message The exception error message
    /// @param location The std::source_location which will be turned into useful debug text
    InexorException(std::string message, std::source_location location = std::source_location::current());
};

/// InexorException for Vulkan specific things.
class VulkanException final : public InexorException {
public:
    /// Default constructor
    /// @param message The exception message
    /// @param result The VkResult value of the Vulkan API call which failed
    /// @param object_name The name of the Vulkan object which was attempted to be created (empty by default)
    /// @param location The std::source_location which will be turned into useful debug text
    /// (``std::source_location::current()`` by default)
    VulkanException(std::string message, VkResult result, std::string object_name = "",
                    std::source_location location = std::source_location::current());
};

} // namespace inexor::vulkan_renderer::tools
