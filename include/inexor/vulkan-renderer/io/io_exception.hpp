#include <stdexcept>

namespace inexor::vulkan_renderer::io {

class IoException : public std::runtime_error {
    // No need to define own constructors.
    using std::runtime_error::runtime_error;
};

} // namespace inexor::vulkan_renderer::io
