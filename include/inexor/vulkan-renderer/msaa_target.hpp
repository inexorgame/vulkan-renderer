#pragma once

#include "inexor/vulkan-renderer/wrapper/image.hpp"

namespace inexor::vulkan_renderer {

struct MSAATarget {
    // The color buffer.
    std::unique_ptr<wrapper::Image> color;

    // The depth buffer.
    std::unique_ptr<wrapper::Image> depth;
};

} // namespace inexor::vulkan_renderer
