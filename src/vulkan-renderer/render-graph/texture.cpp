#include "inexor/vulkan-renderer/render-graph/texture.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Texture::Texture(const Device &device,
                 std::string name,
                 const TextureUsage usage,
                 std::optional<std::function<void()>> on_init,
                 std::optional<std::function<void()>> on_update)
    : m_device(device), m_name(std::move(name)), m_usage(usage), m_on_init(std::move(on_init)),
      m_on_update(std::move(on_update)) {
    if (m_name.empty()) {
        throw std::invalid_argument("[Texture::Texture] Error: Parameter 'name' is empty!");
    }
}

Texture::Texture(const Device &device, std::string name, const TextureUsage usage, const VkFormat format)
    : m_device(device), m_name(std::move(name)), m_usage(usage), m_format(format) {
    if (m_name.empty()) {
        throw std::invalid_argument("[Texture::Texture] Error: Parameter 'name' is empty!");
    }
}

Texture::Texture(Texture &&other) noexcept : m_device(other.m_device) {
    // TODO: FIX me!
}

Texture::~Texture() {
    destroy();
}

void Texture::create() {
    if (const auto result =
            vmaCreateImage(m_device.allocator(), &m_img_ci, &m_alloc_ci, &m_img, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }
    // Assign an internal debug name to this image in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());
    // Set the textures's internal debug name through Vulkan debug utils
    m_device.set_debug_name(m_img, m_name);

    // Set the image in the VkImageViewCreateInfo
    m_img_view_ci.image = m_img;

    if (const auto result = vkCreateImageView(m_device.device(), &m_img_view_ci, nullptr, &m_img_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + m_name + "!", result);
    }
    m_device.set_debug_name(m_img_view, m_name);

    m_sampler = std::make_unique<wrapper::Sampler>(m_device, m_name);
}

void Texture::create(VkImageCreateInfo img_ci, VkImageViewCreateInfo img_view_ci) {
    m_img_ci = std::move(img_ci);
    m_img_view_ci = std::move(img_view_ci);
    create();
}

void Texture::destroy() {
    vkDestroyImageView(m_device.device(), m_img_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_img, m_alloc);
    vmaDestroyBuffer(m_device.allocator(), m_staging_buffer, m_staging_buffer_alloc);
}

void Texture::update(const CommandBuffer &cmd_buf) {
    if (m_img == VK_NULL_HANDLE) {
        throw std::runtime_error("[Texture::execute_update] Error: m_img is VK_NULL_HANDLE!");
    }
    if (m_staging_buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(m_device.allocator(), m_staging_buffer, m_staging_buffer_alloc);
        m_staging_buffer = VK_NULL_HANDLE;
        m_staging_buffer_alloc = VK_NULL_HANDLE;
    }
    const auto staging_buffer_ci = wrapper::make_info<VkBufferCreateInfo>({
        .size = m_texture_data_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    });
    const VmaAllocationCreateInfo staging_buffer_alloc_ci{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    // Create a staging buffer for uploading the texture data
    if (const auto result = vmaCreateBuffer(m_device.allocator(), &staging_buffer_ci, &staging_buffer_alloc_ci,
                                            &m_staging_buffer, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateBuffer failed for staging buffer " + m_name + "!", result);
    }

    cmd_buf.pipeline_image_memory_barrier_before_copy_buffer_to_image(m_img)
        .copy_buffer_to_image(m_staging_buffer, m_img,
                              {
                                  .width = m_img_ci.extent.width,
                                  .height = m_img_ci.extent.height,
                                  .depth = 1,
                              })
        .pipeline_image_memory_barrier_after_copy_buffer_to_image(m_img);

    // TODO: Do we need to create sampler and image view here? or in create()?
    // Update the descriptor image info
    m_descriptor_image_info = VkDescriptorImageInfo{
        .sampler = m_sampler->m_sampler,
        .imageView = m_img_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    // NOTE: The staging buffer needs to stay valid until command buffer finished executing!
    // It will be destroyed either in the destructor or the next time execute_update is called
    // Another option would have been to wrap each call to execute_update() into its own single time
    // command buffer, which would increase the total number of command buffer submissions though
}

void Texture::request_update(void *src_texture_data,
                             const std::size_t src_texture_data_size,
                             VkImageCreateInfo img_ci,
                             VkImageViewCreateInfo img_view_ci) {
    if (src_texture_data == nullptr) {
        throw std::invalid_argument("[Texture::request_update] Error: Parameter 'texture_src_data' is nullptr!");
    }
    if (src_texture_data_size == 0) {
        throw std::invalid_argument("[Texture::request_update] Error: Parameter 'src_texture_data_size' is 0!");
    }
    // NOTE: It is the responsibility of the programmer to make sure the memory this pointer points to is still
    // valid when the actual copy operation for the update is carried out!
    m_texture_data = src_texture_data;
    m_texture_data_size = src_texture_data_size;
    m_img_ci = std::move(img_ci);
    m_img_view_ci = std::move(img_view_ci);
    m_update_requested = true;
}

} // namespace inexor::vulkan_renderer::render_graph
