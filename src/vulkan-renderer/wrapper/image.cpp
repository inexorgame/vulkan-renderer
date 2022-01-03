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
    m_image_layout = m_image_ci.initialLayout;

    m_device.set_debug_marker_name(m_image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_name.c_str());
}

void Image::create_image_view() {
    m_image_view_ci.image = m_image;

    if (const auto result = vkCreateImageView(m_device.device(), &m_image_view_ci, nullptr, &m_image_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + m_name + "!", result);
    }

    m_device.set_debug_marker_name(m_image_view, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, m_name);
}

Image::Image(const Device &device, const VkImageCreateInfo image_ci, const VkImageViewCreateInfo image_view_ci,
             const std::string name)

    : m_device(device), m_image_ci(image_ci), m_image_view_ci(image_view_ci), m_name(name) {

    assert(!m_name.empty());

    create_image();
    create_image_view();
}

void Image::transition_image_layout(const VkCommandBuffer cmd_buf, const VkImageLayout new_layout,
                                    const std::uint32_t miplevel_count, const std::uint32_t layer_count) {

    assert(new_layout != m_image_layout);

    auto barrier = make_info<VkImageMemoryBarrier>();
    barrier.oldLayout = m_image_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // TODO: Expose this as parameter if needed
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = miplevel_count;

    // TODO: Expose this as parameter if needed
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layer_count;

    VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    // TODO: Validate this code block here!
    if (m_image_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (m_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (m_image_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    } else if (m_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    } else if (m_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }

    vkCmdPipelineBarrier(cmd_buf, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    m_image_layout = new_layout;
}

void Image::copy_from_image(const VkCommandBuffer command_buffer, Image &image, const std::uint32_t width,
                            const std::uint32_t height, const std::uint32_t miplevel_count,
                            const std::uint32_t layer_count, const std::uint32_t base_array_layer,
                            const std::uint32_t mip_level) {

    assert(command_buffer);

    image.transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    VkImageSubresourceRange subres_range{};
    subres_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subres_range.baseMipLevel = 0;
    subres_range.levelCount = miplevel_count;
    subres_range.layerCount = layer_count;

    VkImageCopy copy_region{};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.layerCount = layer_count;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.baseArrayLayer = base_array_layer;
    copy_region.dstSubresource.mipLevel = mip_level;
    copy_region.dstSubresource.layerCount = layer_count;
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent.width = width;
    copy_region.extent.height = height;
    copy_region.extent.depth = 1;

    vkCmdCopyImage(command_buffer, image.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    image.transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void Image::copy_from_buffer(const VkCommandBuffer command_buffer, const VkBuffer src_buffer, const std::uint32_t width,
                             const std::uint32_t height) {
    assert(command_buffer);
    assert(src_buffer);

    // TODO: Support mip levels!
    VkBufferImageCopy image_copy{};
    image_copy.bufferOffset = 0;
    image_copy.bufferRowLength = 0;
    image_copy.bufferImageHeight = 0;
    image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_copy.imageSubresource.mipLevel = 0;
    image_copy.imageSubresource.baseArrayLayer = 0;
    image_copy.imageSubresource.layerCount = 1;
    image_copy.imageOffset = {0, 0, 0};
    image_copy.imageExtent = {width, height, 1};

    // TODO: Support mip levels!
    vkCmdCopyBufferToImage(command_buffer, src_buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);
}

Image::Image(Image &&other) noexcept : m_device(other.m_device) {
    m_allocation = std::exchange(other.m_allocation, nullptr);
    m_allocation_info = other.m_allocation_info;
    m_image = std::exchange(other.m_image, VK_NULL_HANDLE);
    m_image_view = std::exchange(other.m_image_view, VK_NULL_HANDLE);
    m_image_layout = other.m_image_layout;
    m_image_ci = other.m_image_ci;
    m_image_view_ci = other.m_image_view_ci;
    m_descriptor = other.m_descriptor;
    m_name = std::move(other.m_name);
}

Image::~Image() {
    vkDestroyImageView(m_device.device(), m_image_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_image, m_allocation);
}

} // namespace inexor::vulkan_renderer::wrapper
