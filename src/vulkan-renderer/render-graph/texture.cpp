#include "inexor/vulkan-renderer/render-graph/texture.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/sampler.hpp"

#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Texture::Texture(const Device &device,
                 std::string name,
                 const TextureUsage usage,
                 const VkFormat format,
                 const std::uint32_t width,
                 const std::uint32_t height,
                 const VkSampleCountFlagBits sample_count,
                 std::optional<std::function<void()>> on_init,
                 std::optional<std::function<void()>> on_check_for_updates)
    : m_device(device), m_name(std::move(name)), m_usage(usage), m_format(format), m_width(width), m_height(height),
      m_sample_count(sample_count), m_on_init(std::move(on_init)),
      m_on_check_for_updates(std::move(on_check_for_updates)) {
    if (m_name.empty()) {
        throw std::invalid_argument("[Texture::Texture] Error: Parameter 'name' is empty!");
    }
    m_img = std::make_unique<Image>(m_device, m_name);

    if (sample_count > VK_SAMPLE_COUNT_1_BIT) {
        m_msaa_img = std::make_unique<Image>(m_device, m_name);
    }
}

Texture::Texture(Texture &&other) noexcept : m_device(other.m_device) {
    // TODO: Implement me!
}

Texture::~Texture() {
    destroy();
}

void Texture::create() {
    auto img_ci = wrapper::make_info<VkImageCreateInfo>({
        .imageType = VK_IMAGE_TYPE_2D,
        .format = m_format,
        .extent =
            {
                .width = m_width,
                .height = m_height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = [&]() -> VkImageUsageFlags {
            switch (m_usage) {
            case TextureUsage::NORMAL: {
                return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                       VK_IMAGE_USAGE_SAMPLED_BIT;
            }
            case TextureUsage::BACK_BUFFER: {
                return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            default: {
                // TextureUsage::DEPTH_STENCIL_BUFFER
                return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
            }
        }(),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    });

    const auto img_view_ci = wrapper::make_info<VkImageViewCreateInfo>({
        // NOTE: .image will be filled by the Texture wrapper
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = m_format,
        .subresourceRange =
            {
                .aspectMask = [&]() -> VkImageAspectFlags {
                    switch (m_usage) {
                    case TextureUsage::DEPTH_STENCIL_BUFFER: {
                        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                    }
                    default: {
                        // TextureUsage::NORMAL and TextureUsage::BACK_BUFFER
                        return VK_IMAGE_ASPECT_COLOR_BIT;
                    }
                    }
                }(),
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    });

    // Create the texture
    m_img->create(img_ci, img_view_ci);

    // If MSAA is enabled, create the MSAA texture as well
    if (m_sample_count > VK_SAMPLE_COUNT_1_BIT) {
        // Just overwrite the sample count and re-use the image create info
        img_ci.samples = m_sample_count;
        m_msaa_img->create(img_ci, img_view_ci);
    }
}

void Texture::destroy() {
    m_img->destroy();
    if (m_msaa_img) {
        m_msaa_img->destroy();
    }
    destroy_staging_buffer();
}

void Texture::destroy_staging_buffer() {
    vmaDestroyBuffer(m_device.allocator(), m_staging_buffer, m_staging_buffer_alloc);
    m_staging_buffer = VK_NULL_HANDLE;
    m_staging_buffer_alloc = VK_NULL_HANDLE;
}

void Texture::update(const CommandBuffer &cmd_buf) {
    if (m_src_texture_data_size == 0) {
        // We can't create buffers of size 0!
        return;
    }
    if (m_staging_buffer != VK_NULL_HANDLE) {
        destroy_staging_buffer();
    }
    const auto staging_buffer_ci = wrapper::make_info<VkBufferCreateInfo>({
        .size = m_width * m_height * m_channels,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    });
    const VmaAllocationCreateInfo staging_buffer_alloc_ci{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    // Create a staging buffer for uploading the texture data
    if (const auto result = vmaCreateBuffer(m_device.allocator(), &staging_buffer_ci, &staging_buffer_alloc_ci,
                                            &m_staging_buffer, &m_staging_buffer_alloc, &m_staging_buffer_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateBuffer failed for staging buffer " + m_name + "!", result);
    }

    const std::string staging_buf_name = "staging:" + m_name;
    // Set the buffer's internal debug name in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_staging_buffer_alloc, staging_buf_name.c_str());
    // Set the buffer's internal debug name through Vulkan debug utils
    m_device.set_debug_name(m_staging_buffer, staging_buf_name);

    cmd_buf.pipeline_image_memory_barrier_before_copy_buffer_to_image(m_img->m_img)
        .copy_buffer_to_image(m_staging_buffer, m_img->m_img,
                              {
                                  .width = m_width,
                                  .height = m_height,
                                  .depth = 1,
                              })
        .pipeline_image_memory_barrier_after_copy_buffer_to_image(m_img->m_img);

    // This is necessary for external textures only, not depth or back buffers used internally in rendergraph
    if (m_usage == TextureUsage::NORMAL) {
        // Update the descriptor image info
        m_descriptor_img_info = VkDescriptorImageInfo{
            .sampler = m_img->m_sampler->m_sampler,
            .imageView = m_img->m_img_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }
    // The update is finished
    m_update_requested = false;

    // NOTE: The staging buffer needs to stay valid until command buffer finished executing!
    // It will be destroyed either in the destructor or the next time execute_update is called.

    // NOTE: Another option would have been to wrap each call to execute_update() into its own single time
    // command buffer, which would increase the total number of command buffer submissions though.
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
