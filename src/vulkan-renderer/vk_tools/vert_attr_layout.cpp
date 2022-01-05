#include "inexor/vulkan-renderer/vk_tools/vert_attr_layout.hpp"

namespace inexor::vulkan_renderer::vk_tools {

VertexAttributeLayout::VertexAttributeLayout(const VkFormat format, const std::size_t size,
                                             const std::uint32_t offset) {
    this->format = format;
    this->size = size;
    this->offset = offset;
}

} // namespace inexor::vulkan_renderer::vk_tools
