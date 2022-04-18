#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

void Image::create_image() {
    VmaAllocationCreateInfo vma_allocation_ci{};
    vma_allocation_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

#if VMA_RECORDING_ENABLED
    vma_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    vma_allocation_ci.pUserData = m_name.data();
#else
    vma_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif

    if (const auto result = vmaCreateImage(m_device.allocator(), &m_image_ci, &vma_allocation_ci, &m_image,
                                           &m_allocation, &m_allocation_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }

    // Keep track of image layouts for image transitioning
    descriptor_image_info.imageLayout = m_image_ci.initialLayout;

    m_device.set_debug_marker_name(m_image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_name.c_str());
}

void Image::create_image_view() {
    m_image_view_ci.image = m_image;

    if (const auto result = vkCreateImageView(m_device.device(), &m_image_view_ci, nullptr, &m_image_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + m_name + "!", result);
    }

    m_device.set_debug_marker_name(m_image_view, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, m_name);

    descriptor_image_info.imageView = m_image_view;
}

Image::Image(const Device &device, const VkImageCreateInfo image_ci, const VkImageViewCreateInfo image_view_ci,
             const std::string name)

    : m_device(device), m_image_ci(image_ci), m_image_view_ci(image_view_ci), m_name(name) {

    assert(!m_name.empty());

    create_image();
    create_image_view();
}

void Image::change_image_layout(const wrapper::CommandBuffer &cmd_buf, const VkImageLayout old_layout,
                                const VkImageLayout new_layout, const std::uint32_t miplevel_count,
                                const std::uint32_t layer_count, const std::uint32_t base_mip_level,
                                const std::uint32_t base_array_layer) {

    auto barrier = make_info<VkImageMemoryBarrier>();
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = base_mip_level;
    barrier.subresourceRange.levelCount = miplevel_count;
    barrier.subresourceRange.baseArrayLayer = base_array_layer;
    barrier.subresourceRange.layerCount = layer_count;

    VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    // TODO: Validate this code block here!
    if (descriptor_image_info.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (descriptor_image_info.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (descriptor_image_info.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    } else if (descriptor_image_info.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    } else if (descriptor_image_info.imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }

    cmd_buf.pipeline_barrier(source_stage, destination_stage, barrier);
}

void Image::copy_from_buffer(const wrapper::CommandBuffer &cmd_buf, const VkBuffer src_buffer,
                             const std::uint32_t width, const std::uint32_t height) {
    assert(src_buffer);
    assert(width > 0);
    assert(height > 0);

    // TODO: Support mip levels!
    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageOffset = {0, 0, 0};
    copy_region.imageExtent = {width, height, 1};

    cmd_buf.copy_buffer_to_image(src_buffer, m_image, copy_region);
}

Image::Image(Image &&other) noexcept : m_device(other.m_device) {
    m_allocation = std::exchange(other.m_allocation, nullptr);
    m_allocation_info = other.m_allocation_info;
    m_image = std::exchange(other.m_image, VK_NULL_HANDLE);
    m_image_view = std::exchange(other.m_image_view, VK_NULL_HANDLE);
    descriptor_image_info = other.descriptor_image_info;
    m_image_ci = other.m_image_ci;
    m_image_view_ci = other.m_image_view_ci;
    m_name = std::move(other.m_name);
}

Image::~Image() {
    vkDestroyImageView(m_device.device(), m_image_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_image, m_allocation);
}

} // namespace inexor::vulkan_renderer::wrapper
