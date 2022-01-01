#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"

#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::texture {

GpuTexture::GpuTexture(const wrapper::Device &device, const void *texture_data, const std::size_t texture_size,
                       const VkImageCreateInfo image_ci, const VkImageViewCreateInfo image_view_ci,
                       const VkSamplerCreateInfo sampler_ci, const std::string name)

    : GpuTexture(device, image_ci, image_view_ci, sampler_ci, name) {

    upload_texture_data(texture_data, texture_size);
}

GpuTexture::GpuTexture(const wrapper::Device &device, VkImageCreateInfo image_ci, VkImageViewCreateInfo image_view_ci,
                       VkSamplerCreateInfo sampler_ci, std::string name)

    : m_device(device), m_image_ci(image_ci), m_image_view_ci(image_view_ci), m_sampler_ci(sampler_ci), m_name(name) {

    m_image = std::make_unique<wrapper::Image>(m_device, image_ci, image_view_ci, sampler_ci, name);
}

GpuTexture::GpuTexture(const wrapper::Device &device, const CpuTexture &cpu_texture, const VkImageCreateInfo image_ci,
                       const VkImageViewCreateInfo image_view_ci, const VkSamplerCreateInfo sampler_ci)

    : GpuTexture(device, cpu_texture.data(), cpu_texture.data_size(), image_ci, image_view_ci, sampler_ci,
                 cpu_texture.name()) {}

void GpuTexture::upload_texture_data(const void *texture_data, const std::size_t texture_size) {
    assert(texture_data);
    assert(texture_size > 0);

    wrapper::StagingBuffer texture_staging_buffer(m_device, m_name, texture_size, texture_data, texture_size);

    wrapper::OnceCommandBuffer copy_command(m_device, m_device.graphics_queue(),
                                            m_device.graphics_queue_family_index());

    copy_command.create_command_buffer();
    copy_command.start_recording();
    {
        m_image->transition_image_layout(copy_command.command_buffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        m_image->copy_from_buffer(copy_command.command_buffer(), texture_staging_buffer.buffer(),
                                  m_image_ci.extent.width, m_image_ci.extent.height);

        m_image->transition_image_layout(copy_command.command_buffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    copy_command.end_recording_and_submit_command();
}

GpuTexture::GpuTexture(GpuTexture &&other) noexcept : m_device(other.m_device) {
    m_image = std::exchange(other.m_image, nullptr);
}

} // namespace inexor::vulkan_renderer::texture
