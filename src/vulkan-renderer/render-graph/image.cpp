#include "inexor/vulkan-renderer/render-graph/image.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/sampler.hpp"

using inexor::vulkan_renderer::tools::VulkanException;

namespace inexor::vulkan_renderer::render_graph {

Image::Image(const Device &device, std::string name) : m_device(device), m_name(std::move(name)) {}

Image::Image(Image &&other) noexcept : m_device(other.m_device), m_alloc_ci(other.m_alloc_ci) {
    // TODO: Check me!
    other.m_name = std::move(other.m_name);
    m_img = std::exchange(other.m_img, VK_NULL_HANDLE);
    m_img_view = std::exchange(other.m_img_view, VK_NULL_HANDLE);
    m_alloc = std::exchange(other.m_alloc, VK_NULL_HANDLE);
    m_alloc_info = other.m_alloc_info;
}

Image::~Image() {
    destroy();
}

void Image::create(VkImageCreateInfo img_ci, VkImageViewCreateInfo img_view_ci) {
    m_img_ci = std::move(img_ci);
    m_img_view_ci = std::move(img_view_ci);

    // Create the image
    if (const auto result =
            vmaCreateImage(m_device.allocator(), &m_img_ci, &m_alloc_ci, &m_img, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed!", result, m_name);
    }
    // Set the image's internal debug name in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());
    // Set the image's internal debug name through Vulkan debug utils
    m_device.set_debug_name(m_img, m_name);

    // Set the image in the VkImageViewCreateInfo
    m_img_view_ci.image = m_img;

    // Create the image view
    if (const auto result = vkCreateImageView(m_device.device(), &m_img_view_ci, nullptr, &m_img_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed!", result, m_name);
    }
    m_device.set_debug_name(m_img_view, m_name);
}

void Image::destroy() {
    // Destroy the image view
    vkDestroyImageView(m_device.device(), m_img_view, nullptr);
    m_img_view = VK_NULL_HANDLE;

    // Destroy the image
    vmaDestroyImage(m_device.allocator(), m_img, m_alloc);
    m_img = VK_NULL_HANDLE;
    m_alloc = VK_NULL_HANDLE;
}

} // namespace inexor::vulkan_renderer::render_graph
