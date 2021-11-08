#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// TODO: Should this actually be in wrapper?
struct TextureAttributes {
    VkFormat format{VK_FORMAT_R8G8B8A8_UNORM};
    std::string name;
    std::uint32_t width{0};
    std::uint32_t height{0};
    std::uint32_t channels{0};
    std::uint32_t mip_levels{0};
};

} // namespace inexor::vulkan_renderer::wrapper
