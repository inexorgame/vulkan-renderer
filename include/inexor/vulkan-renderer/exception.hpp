#pragma once

#include <vulkan/vulkan_core.h>

#include <source_location>
#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer {

/// A custom base class for exceptions
class InexorException : public std::runtime_error {
public:
    // There is no need to define our own constructors
    using std::runtime_error::runtime_error;
};

/// Custom exception class for Vulkan related exceptions
class VulkanException final : public InexorException {
public:
    /// Default constructor
    /// Here we are using `C++20 source location feature <https://en.cppreference.com/w/cpp/utility/source_location>`__
    /// @param message The exception message
    /// @param result The VkResult value of the Vulkan API call which failed
    /// @param location The source location
    VulkanException(std::string message, VkResult result,
                    std::source_location location = std::source_location::current());
};

} // namespace inexor::vulkan_renderer
