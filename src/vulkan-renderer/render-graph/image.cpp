#include "inexor/vulkan-renderer/render-graph/image.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/sampler.hpp"

namespace inexor::vulkan_renderer::render_graph {

Image::Image(const Device &device) : m_device(device) {}

Image::~Image() {
    destroy();
}

void Image::create(const VkImageCreateInfo img_ci, const VkImageViewCreateInfo img_view_ci) {
    m_img_ci = img_ci;
    m_img_view_ci = img_view_ci;

    // Create the image
    if (const auto result = vmaCreateImage(m_device.allocator(), &img_ci, &m_alloc_ci, &m_img, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());
    m_device.set_debug_name(m_img, m_name);

    // Set the image in the VkImageViewCreateInfo
    m_img_view_ci.image = m_img;

    // Create the image view
    if (const auto result = vkCreateImageView(m_device.device(), &m_img_view_ci, nullptr, &m_img_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + m_name + "!", result);
    }
    m_device.set_debug_name(m_img_view, m_name);

    // TODO: Pass VkSamplerCreateInfo as parameter from Image wrapper to the Sampler wrapper
    // Create a default sampler
    m_sampler = std::make_unique<Sampler>(m_device, "Default");
}

void Image::destroy() {
    vkDestroyImageView(m_device.device(), m_img_view, nullptr);
    m_img_view = VK_NULL_HANDLE;
    vmaDestroyImage(m_device.allocator(), m_img, m_alloc);
    m_img = VK_NULL_HANDLE;
    m_sampler.reset();
    m_sampler = nullptr;
}

} // namespace inexor::vulkan_renderer::render_graph
