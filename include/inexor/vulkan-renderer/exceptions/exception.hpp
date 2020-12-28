#pragma once

#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer::exceptions {

/// @brief A custom base class for exceptions
class Exception : public std::runtime_error {
public:
    // No need to define own constructors.
    using std::runtime_error::runtime_error;
};

} // namespace inexor::vulkan_renderer::exceptions
