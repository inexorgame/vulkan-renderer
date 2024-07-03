#include "inexor/vulkan-renderer/render-graph/texture.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Texture::Texture(const Device &device,
                 std::string name,
                 const TextureUsage usage,
                 const VkFormat format,
                 std::optional<std::function<void()>> on_init,
                 std::optional<std::function<void()>> on_update)
    : m_device(device), m_name(std::move(name)), m_usage(usage), m_format(format), m_on_init(std::move(on_init)),
      m_on_update(std::move(on_update)) {}

Texture::Texture(Texture &&other) noexcept : m_device(other.m_device) {
    // TODO: FIX me!
}

void Texture::create_texture(const VkImageCreateInfo &img_ci,
                             const VkImageViewCreateInfo &img_view_ci,
                             const VmaAllocationCreateInfo &alloc_ci) {
    if (const auto result = vmaCreateImage(m_device.allocator(), &img_ci, &alloc_ci, &m_img, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }
    // Assign an internal debug name to this image in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());
    // Set the textures's internal debug name through Vulkan debug utils
    m_device.set_debug_name(m_img, m_name);

    // Fill in the image that was created and the format of the image
    auto filled_img_view_ci = img_view_ci;
    filled_img_view_ci.image = m_img;
    filled_img_view_ci.format = img_ci.format;

    if (const auto result = vkCreateImageView(m_device.device(), &filled_img_view_ci, nullptr, &m_img_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + m_name + "!", result);
    }
    m_device.set_debug_name(m_img_view, m_name);
}

void Texture::request_update(void *texture_src_data, const std::size_t src_texture_data_size) {
    if (texture_src_data == nullptr) {
        throw std::invalid_argument("[Texture::request_update] Error: Parameter 'texture_src_data' is nullptr!");
    }
    if (src_texture_data_size == 0) {
        throw std::invalid_argument("[Texture::request_update] Error: Parameter 'src_texture_data_size' is 0!");
    }
    // NOTE: It is the responsibility of the programmer to make sure the memory this pointer points to is still
    // valid when the actual copy operation for the update is carried out!
    m_texture_data = texture_src_data;
    m_texture_data_size = src_texture_data_size;
}

} // namespace inexor::vulkan_renderer::render_graph
