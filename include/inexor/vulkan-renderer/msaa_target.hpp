#pragma once

#include "inexor/vulkan-renderer/image_buffer.hpp"

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
