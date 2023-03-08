#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Image::Image(const Device &device, const VkImageCreateInfo &img_ci, const VmaAllocationCreateInfo &alloc_ci,
             const VkImageViewCreateInfo &img_view_ci, std::string name)
    : m_device(device), m_format(img_ci.format), m_name(std::move(name)) {
    assert(!m_name.empty());
    m_device.create_image(img_ci, alloc_ci, &m_img, &m_alloc, &m_alloc_info, m_name);

    // We need to set the image in the image view create info here
    VkImageViewCreateInfo filled_img_view_ci = img_view_ci;
    filled_img_view_ci.image = m_img;
    filled_img_view_ci.format = img_ci.format;
    m_device.create_image_view(filled_img_view_ci, &m_img_view, m_name);
}

Image::Image(Image &&other) noexcept : m_device(other.m_device) {
    m_format = other.m_format;
    m_alloc = other.m_alloc;
    m_alloc_info = other.m_alloc_info;
    m_img = std::exchange(other.m_img, VK_NULL_HANDLE);
    m_img_view = std::exchange(other.m_img_view, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

Image::~Image() {
    vkDestroyImageView(m_device.device(), m_img_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_img, m_alloc);
}

} // namespace inexor::vulkan_renderer::wrapper
