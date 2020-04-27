#pragma once

#include "inexor/vulkan-renderer/image_buffer.hpp"

namespace inexor::vulkan_renderer {

struct MSAATarget {
    // The color buffer.
    ImageBuffer color;

    // The depth buffer.
    ImageBuffer depth;
};

} // namespace inexor::vulkan_renderer
