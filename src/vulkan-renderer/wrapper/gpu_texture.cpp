﻿#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

GpuTexture::GpuTexture(const wrapper::Device &device, const CpuTexture &cpu_texture)
    : m_device(device), m_texture_width(cpu_texture.width()), m_texture_height(cpu_texture.height()),
      m_texture_channels(cpu_texture.channels()), m_mip_levels(cpu_texture.mip_levels()), m_name(cpu_texture.name()) {
    create_texture(cpu_texture.data(), cpu_texture.data_size());
}

GpuTexture::GpuTexture(const wrapper::Device &device, void *data, const std::size_t data_size, const int texture_width,
                       const int texture_height, const int texture_channels, const int mip_levels, std::string name)
    : m_device(device), m_texture_width(texture_width), m_texture_height(texture_height),
      m_texture_channels(texture_channels), m_mip_levels(mip_levels), m_name(std::move(name)) {
    create_texture(data, data_size);
}

GpuTexture::GpuTexture(GpuTexture &&other) noexcept : m_device(other.m_device) {
    m_texture_image = std::exchange(other.m_texture_image, nullptr);
    m_name = std::move(other.m_name);
    m_texture_width = other.m_texture_width;
    m_texture_height = other.m_texture_height;
    m_texture_channels = other.m_texture_channels;
    m_texture_format = other.m_texture_format;
    m_mip_levels = other.m_mip_levels;
    m_sampler = std::exchange(other.m_sampler, nullptr);
}

GpuTexture::~GpuTexture() {
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
}

void GpuTexture::create_texture(void *texture_data, const std::size_t texture_size) {
    const auto img_view = wrapper::make_info<VkImageCreateInfo>({
        .imageType = VK_IMAGE_TYPE_2D,
        .format = m_texture_format,
        .extent{
            .width = static_cast<std::uint32_t>(m_texture_width),
            .height = static_cast<std::uint32_t>(m_texture_height),
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    });

    const auto img_view_ci = wrapper::make_info<VkImageViewCreateInfo>({
        // Note that .image and .format are set by wrapper::Image itself
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    });

    const VmaAllocationCreateInfo alloc_ci{
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    m_texture_image = std::make_unique<wrapper::Image>(m_device, img_view, alloc_ci, img_view_ci, "GpuTexture");

    const VkBufferImageCopy copy_region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
        .imageOffset = {0, 0, 0},
        // Because stb_image stored the texture's width and height as a normal int, we need a cast here
        .imageExtent = {static_cast<uint32_t>(m_texture_width), static_cast<uint32_t>(m_texture_height), 1},
    };

    m_device.execute(m_name, [&](const CommandBuffer &cmd_buf) {
        cmd_buf
            .change_image_layout(m_texture_image->image(), VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            .copy_buffer_to_image(texture_data, static_cast<VkDeviceSize>(texture_size), m_texture_image->image(),
                                  copy_region, m_name)
            .change_image_layout(m_texture_image->image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    create_texture_sampler();
}

void GpuTexture::create_texture_sampler() {
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(m_device.physical_device(), &device_features);

    VkPhysicalDeviceProperties graphics_card_properties;
    vkGetPhysicalDeviceProperties(m_device.physical_device(), &graphics_card_properties);

    const auto sampler_ci = make_info<VkSamplerCreateInfo>({
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    });

    m_device.create_sampler(sampler_ci, &m_sampler, m_name);
}

} // namespace inexor::vulkan_renderer::wrapper
