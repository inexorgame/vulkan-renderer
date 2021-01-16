#pragma once

#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <memory>

namespace inexor::vulkan_renderer {

struct MSAATarget {
    // The color buffer.
    std::unique_ptr<wrapper::Image> m_color{nullptr};

    // The depth buffer.
    std::unique_ptr<wrapper::Image> m_depth{nullptr};
};

} // namespace inexor::vulkan_renderer
