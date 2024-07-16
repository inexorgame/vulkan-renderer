#include "inexor/vulkan-renderer/exception.hpp"

namespace inexor::vulkan_renderer::io {

class IoException : public InexorException {
    using InexorException::InexorException;
};

} // namespace inexor::vulkan_renderer::io
