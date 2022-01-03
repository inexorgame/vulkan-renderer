#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>

namespace inexor::vulkan_renderer::vk_tools {

///
[[nodiscard]] VkImageCreateInfo fill_image_ci(VkFormat format, std::uint32_t width, std::uint32_t height,
                                              std::uint32_t miplevel_count,
                                              VkImageUsageFlags usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT);

///
///
[[nodiscard]] VkImageViewCreateInfo fill_image_view_ci(VkFormat format);

///
///
[[nodiscard]] VkSamplerCreateInfo fill_sampler_ci();
} // namespace inexor::vulkan_renderer::vk_tools
