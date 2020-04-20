#pragma once

#include "vulkan-renderer/image-buffer/image_buffer.hpp"

namespace inexor::vulkan_renderer {

///
///
///
struct InexorMSAATarget {
    // The color buffer.
    InexorImageBuffer color;

    // The depth buffer.
    InexorImageBuffer depth;
};

} // namespace inexor::vulkan_renderer
