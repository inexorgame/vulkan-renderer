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
                 const VkFormat format,
                 std::optional<std::function<void()>> on_init,
                 std::optional<std::function<void()>> on_update)
    : m_device(device), m_name(std::move(name)), m_texture_usage(usage), m_format(format),
      m_on_init(std::move(on_init)), m_on_update(std::move(on_update)) {
    if (m_name.empty()) {
        throw std::invalid_argument("[Texture::Texture] Error: Parameter 'name' is empty!");
    }
}

void Texture::destroy() {
    // TODO: Do we need this here?
    m_img->destroy();
}

void Texture::update(const CommandBuffer &cmd_buf) {
#if 0
    // TODO: Validate parameters again before update!
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
#endif
}

void Texture::request_update(void *src_texture_data, const std::size_t src_texture_data_size) {
    if (src_texture_data == nullptr) {
        throw std::invalid_argument("[Texture::request_update] Error: Parameter 'texture_src_data' is nullptr!");
    }
    if (src_texture_data_size == 0) {
        throw std::invalid_argument("[Texture::request_update] Error: Parameter 'src_texture_data_size' is 0!");
    }
    m_update_requested = true;
    m_src_texture_data = src_texture_data;
    m_src_texture_data_size = src_texture_data_size;
}

} // namespace inexor::vulkan_renderer::render_graph
