#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/vk_tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <ktx.h>
#include <ktxvulkan.h>

#include <algorithm>
#include <cassert>

namespace inexor::vulkan_renderer::cubemap {

GpuCubemap::GpuCubemap(const wrapper::Device &device, VkImageCreateInfo image_ci, std::string name) : m_device(device) {
    // TODO: Implement
}

GpuCubemap::GpuCubemap(const wrapper::Device &device, const texture::CpuTexture &cpu_cubemap,
                       const VkImageCreateInfo image_ci)

    : m_device(device), m_name(cpu_cubemap.name()) {

    assert(cpu_cubemap.ktx_wrapper() != nullptr);

    wrapper::StagingBuffer texture_staging_buffer(m_device, cpu_cubemap.ktx_texture_data_size(),
                                                  cpu_cubemap.ktx_texture_data(), m_name);

    // Setup buffer copy regions for each face including all of its mip levels
    std::vector<VkBufferImageCopy> copy_regions;

    copy_regions.reserve(cpu_cubemap.mip_levels() * FACE_COUNT);

    for (std::uint32_t face = 0; face < FACE_COUNT; face++) {
        for (std::uint32_t mip_level = 0; mip_level < cpu_cubemap.mip_levels(); mip_level++) {
            ktx_size_t offset = 0;

            if (const auto result = ktxTexture_GetImageOffset(cpu_cubemap.ktx_wrapper(), mip_level, 0, 0, &offset);
                result != KTX_SUCCESS) {
                throw KtxException("Error: ktxTexture_GetImageOffset failed!", result);
            }

            VkBufferImageCopy copy_region{};
            copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copy_region.imageSubresource.mipLevel = mip_level;
            copy_region.imageSubresource.baseArrayLayer = face;
            copy_region.imageSubresource.layerCount = 1;
            copy_region.imageExtent.width = cpu_cubemap.ktx_wrapper()->baseWidth >> mip_level;
            copy_region.imageExtent.height = cpu_cubemap.ktx_wrapper()->baseHeight >> mip_level;
            copy_region.imageExtent.depth = 1;
            copy_region.bufferOffset = offset;

            copy_regions.push_back(copy_region);
        }
    }

    // TODO: go on...
}

void GpuCubemap::copy_from_image(const VkCommandBuffer cmd_buf, const VkImage source_image, const std::uint32_t face,
                                 const std::uint32_t mip_level, const std::uint32_t width, const std::uint32_t height) {
    VkImageCopy region{};

    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.layerCount = 1;
    region.srcOffset = {0, 0, 0};

    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.baseArrayLayer = face;
    region.dstSubresource.mipLevel = mip_level;
    region.dstSubresource.layerCount = 1;
    region.dstOffset = {0, 0, 0};

    region.extent.width = width;
    region.extent.height = height;
    region.extent.depth = 1;

    vkCmdCopyImage(cmd_buf, source_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image->image(),
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

} // namespace inexor::vulkan_renderer::cubemap
