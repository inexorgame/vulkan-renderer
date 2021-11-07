#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

void Image::create_image(const VkImageCreateInfo image_ci) {
    VmaAllocationCreateInfo vma_allocation_ci{};
    vma_allocation_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

#if VMA_RECORDING_ENABLED
    vma_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    vma_allocation_ci.pUserData = m_name.data();
#else
    vma_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif

    if (const auto result = vmaCreateImage(m_device.allocator(), &image_ci, &vma_allocation_ci, &m_image, &m_allocation,
                                           &m_allocation_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_name);

    // Keep track of image layouts for image transitioning
    m_image_layout = image_ci.initialLayout;
}

void Image::create_image_view(const VkImageViewCreateInfo image_view_ci) {
    if (const auto result = vkCreateImageView(m_device.device(), &image_view_ci, nullptr, &m_image_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + m_name + "!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_image_view, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, m_name);
}

void Image::update_descriptor() {
    // TODO: Fix this!
    // m_descriptor.sampler = m_sampler;
    m_descriptor.imageView = m_image_view;
    m_descriptor.imageLayout = m_image_layout;
}

Image::Image(const Device &device, const VkImageCreateInfo image_ci, const VkImageViewCreateInfo image_view_ci,
             std::string name)

    : m_device(device), m_format(image_ci.format), m_name(std::move(name)) {

    assert(device.device());
    assert(device.physical_device());
    assert(device.allocator());
    assert(!name.empty());

    create_image(image_ci);
    create_image_view(image_view_ci);
}

Image::Image(const Device &device, const VkImageCreateFlags flags, const VkImageType image_type, const VkFormat format,
             const std::uint32_t width, const std::uint32_t height, const std::uint32_t miplevel_count,
             const std::uint32_t array_layer_count, const VkSampleCountFlagBits sample_count_flags,
             const VkImageUsageFlags image_usage_flags, const VkImageViewType image_view_type,
             const VkComponentMapping view_components, const VkImageAspectFlags image_aspect_flags, std::string name)

    : m_device(device), m_format(format), m_name(std::move(name)) {

    assert(device.device());
    assert(device.physical_device());
    assert(device.allocator());
    assert(array_layer_count > 0);
    assert(miplevel_count > 0);
    assert(width > 0);
    assert(height > 0);
    assert(!m_name.empty());

    auto image_ci = make_info<VkImageCreateInfo>();
    image_ci.flags = flags;
    image_ci.imageType = image_type;
    image_ci.format = format;
    image_ci.extent.width = width;
    image_ci.extent.height = height;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = miplevel_count;
    image_ci.arrayLayers = array_layer_count;
    image_ci.samples = sample_count_flags;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = image_usage_flags;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    create_image(image_ci);

    auto image_view_ci = make_info<VkImageViewCreateInfo>();
    image_view_ci.image = m_image;
    image_view_ci.viewType = image_view_type;
    image_view_ci.format = format;
    image_view_ci.components = view_components;
    image_view_ci.subresourceRange.aspectMask = image_aspect_flags;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = miplevel_count;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount = array_layer_count;

    create_image_view(image_view_ci);

    device.set_debug_marker_name(m_image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_name.c_str());
}

Image::Image(const Device &device, const VkImageCreateFlags flags, const VkImageType image_type, const VkFormat format,
             const std::uint32_t width, const std::uint32_t height, const std::uint32_t miplevel_count,
             const std::uint32_t layer_count, const VkSampleCountFlagBits sample_count,
             const VkImageUsageFlags image_usage_flags, const VkImageViewType image_view_type,
             const VkImageAspectFlags aspect_mask, std::string name)

    : Image(device, flags, image_type, format, width, height, miplevel_count, layer_count, sample_count,
            image_usage_flags, image_view_type, {}, aspect_mask, name) {}

Image::Image(const Device &device, const VkImageType image_type, const VkFormat format, const std::uint32_t width,
             const std::uint32_t height, const std::uint32_t miplevel_count, const std::uint32_t layer_count,
             const VkSampleCountFlagBits sample_count_flags, const VkImageUsageFlags image_usage_flags,
             const VkImageViewType image_view_type, const VkImageAspectFlags image_aspect_flags, std::string name)

    : Image(device, {}, image_type, format, width, height, miplevel_count, layer_count, sample_count_flags,
            image_usage_flags, image_view_type, {}, image_aspect_flags, name) {}

Image::Image(const Device &device, const VkFormat format, const std::uint32_t width, const std::uint32_t height,
             const VkImageUsageFlags image_usage_flags, const VkImageAspectFlags image_aspect_flags, std::string name)

    : Image(device, {}, VK_IMAGE_TYPE_2D, format, width, height, 1, 1, VK_SAMPLE_COUNT_1_BIT, image_usage_flags,
            VK_IMAGE_VIEW_TYPE_2D, {}, image_aspect_flags, name) {}

Image::Image(const Device &device, const VkImageCreateFlags image_create_flags, const VkFormat format,
             const std::uint32_t width, const std::uint32_t height, const std::uint32_t miplevel_count,
             const std::uint32_t array_layer_count, const VkSampleCountFlagBits sample_count_flags,
             const VkImageUsageFlags image_usage_flags, std::string name)

    : Image(device, image_create_flags, VK_IMAGE_TYPE_2D, format, width, height, miplevel_count, array_layer_count,
            VK_SAMPLE_COUNT_1_BIT, image_usage_flags, VK_IMAGE_VIEW_TYPE_CUBE, {}, VK_IMAGE_ASPECT_COLOR_BIT, name) {}

void Image::transition_image_layout(const VkCommandBuffer cmd_buf, const VkImageLayout new_layout) {

    auto barrier = make_info<VkImageMemoryBarrier>();
    barrier.oldLayout = m_image_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

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

void Image::transition_image_layout(const VkImageLayout new_layout) {

    OnceCommandBuffer image_transition_change(m_device, m_device.graphics_queue(),
                                              m_device.graphics_queue_family_index());

    image_transition_change.create_command_buffer();
    image_transition_change.start_recording();

    transition_image_layout(image_transition_change.command_buffer(), new_layout);

    image_transition_change.end_recording_and_submit_command();
}

void Image::place_pipeline_barrier(const VkImageLayout new_layout, const VkAccessFlags src_access_mask,
                                   const VkAccessFlags dest_access_mask,
                                   const VkImageSubresourceRange subresource_range) {

    wrapper::OnceCommandBuffer cmd_buf(m_device);

    cmd_buf.create_command_buffer();
    cmd_buf.start_recording();

    assert(m_image);
    assert(src_access_mask != dest_access_mask);

    auto mem_barrier = make_info<VkImageMemoryBarrier>();
    mem_barrier.image = m_image;
    mem_barrier.oldLayout = m_image_layout;
    mem_barrier.newLayout = new_layout;
    mem_barrier.srcAccessMask = src_access_mask;
    mem_barrier.dstAccessMask = dest_access_mask;
    mem_barrier.subresourceRange = subresource_range;

    vkCmdPipelineBarrier(cmd_buf.command_buffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &mem_barrier);

    cmd_buf.end_recording_and_submit_command();

    m_image_layout = new_layout;
}

void Image::place_pipeline_barrier(const VkCommandBuffer command_buffer, const VkImageLayout new_layout,
                                   const VkAccessFlags src_access_mask, const VkAccessFlags dest_access_mask,
                                   const VkImageSubresourceRange subresource_range) {

    assert(m_image);
    assert(command_buffer);
    assert(src_access_mask != dest_access_mask);

    auto mem_barrier = make_info<VkImageMemoryBarrier>();
    mem_barrier.image = m_image;
    mem_barrier.oldLayout = m_image_layout;
    mem_barrier.newLayout = new_layout;
    mem_barrier.srcAccessMask = src_access_mask;
    mem_barrier.dstAccessMask = dest_access_mask;
    mem_barrier.subresourceRange = subresource_range;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &mem_barrier);

    m_image_layout = new_layout;
}

void Image::copy_from_image(const VkCommandBuffer command_buffer, Image &image, const std::uint32_t width,
                            const std::uint32_t height, const std::uint32_t miplevel_count,
                            const std::uint32_t layer_count, const std::uint32_t base_array_layer,
                            const std::uint32_t mip_level) {

    assert(command_buffer);

    image.transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    // TODO: Support mip levels!
    VkImageSubresourceRange subres_range{};
    subres_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subres_range.baseMipLevel = 0;
    subres_range.levelCount = miplevel_count;
    subres_range.layerCount = layer_count;

    // TODO: Support mip levels!
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

    // TODO: Support mip levels!
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
    m_allocation = other.m_allocation;
    m_allocation_info = other.m_allocation_info;
    m_image = other.m_image;
    m_format = other.m_format;
    m_image_view = other.m_image_view;
    m_name = std::move(other.m_name);
}

Image::~Image() {
    vkDestroyImageView(m_device.device(), m_image_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_image, m_allocation);
}

} // namespace inexor::vulkan_renderer::wrapper
