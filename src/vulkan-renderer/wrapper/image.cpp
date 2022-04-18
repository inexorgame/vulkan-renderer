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

Image::Image(Image &&other) noexcept : m_device(other.m_device) {
    m_allocation = std::exchange(other.m_allocation, nullptr);
    m_allocation_info = other.m_allocation_info;
    m_image = std::exchange(other.m_image, VK_NULL_HANDLE);
    m_image_view = std::exchange(other.m_image_view, VK_NULL_HANDLE);
    m_image_ci = other.m_image_ci;
    m_image_view_ci = other.m_image_view_ci;
    m_name = std::move(other.m_name);
}

Image::~Image() {
    vkDestroyImageView(m_device.device(), m_image_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_image, m_allocation);
}

} // namespace inexor::vulkan_renderer::wrapper
