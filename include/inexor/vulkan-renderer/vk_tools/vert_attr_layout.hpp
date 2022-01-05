#pragma once

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>

namespace inexor::vulkan_renderer::vk_tools {

struct VertexAttributeLayout {
    VkFormat format;
    std::size_t size;
    std::uint32_t offset;

    VertexAttributeLayout(VkFormat format, std::size_t size, std::uint32_t offset);
};

} // namespace inexor::vulkan_renderer::vk_tools