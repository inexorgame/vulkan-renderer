#include "inexor/vulkan-renderer/vk_tools/fill_vk_struct.hpp"

#include <cassert>

namespace inexor::vulkan_renderer::vk_tools {

VkImageCreateInfo fill_image_ci(const VkFormat format, const std::uint32_t width, const std::uint32_t height,
                                const std::uint32_t miplevel_count, const VkImageUsageFlags usage_flags) {
    VkImageCreateInfo image_ci{};
    image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format;
    image_ci.mipLevels = miplevel_count;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.extent = {width, height, 1};
    image_ci.usage = usage_flags;
    return image_ci;
}

VkImageViewCreateInfo fill_image_view_ci(const VkFormat format) {
    VkImageViewCreateInfo ret = {};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ret.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ret.format = format;
    ret.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    ret.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    ret.subresourceRange.levelCount = 1;
    // ret.image = image;
    return ret;
}

VkSamplerCreateInfo fill_sampler_ci() {
    VkSamplerCreateInfo ret = {};
    ret.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    ret.magFilter = VK_FILTER_LINEAR;
    ret.minFilter = VK_FILTER_LINEAR;
    ret.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    ret.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    ret.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    ret.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    ret.mipLodBias = 0.0f;
    ret.compareOp = VK_COMPARE_OP_NEVER;
    ret.minLod = 0.0f;
    ret.maxLod = 0.0f;
    ret.maxAnisotropy = 1.0f;
    return ret;
}

} // namespace inexor::vulkan_renderer::vk_tools
