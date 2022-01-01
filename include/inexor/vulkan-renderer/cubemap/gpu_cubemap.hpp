#pragma once

#include "inexor/vulkan-renderer/texture/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::cubemap {

class GpuCubemap {
private:
    const wrapper::Device &m_device;

    // Each cube has 6 faces
    static constexpr std::uint32_t FACE_COUNT{6};

    std::unique_ptr<wrapper::Image> m_image;

    VkImageCreateInfo m_image_ci;
    VkImageViewCreateInfo m_image_view_ci;

    std::string m_name;

public:
    GpuCubemap(const wrapper::Device &device, VkImageCreateInfo image_ci, std::string name);

    GpuCubemap(const wrapper::Device &device, const texture::CpuTexture &cpu_cubemap, VkImageCreateInfo image_ci);

    void copy_from_image(VkCommandBuffer cmd_buf, VkImage source_image, std::uint32_t face, std::uint32_t mip_level,
                         std::uint32_t width, std::uint32_t height);

    [[nodiscard]] std::uint32_t width() const {
        return m_image_ci.extent.width;
    }

    [[nodiscard]] std::uint32_t height() const {
        return m_image_ci.extent.height;
    }

    [[nodiscard]] std::uint32_t mip_levels() const {
        return m_image_ci.mipLevels;
    }

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    [[nodiscard]] const auto &image_wrapper() const {
        return m_image;
    }

    [[nodiscard]] VkSampler sampler() const {
        return m_image->sampler();
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_image->image_view();
    }

    [[nodiscard]] VkDescriptorImageInfo descriptor() const {
        return m_image->descriptor();
    }
};

} // namespace inexor::vulkan_renderer::cubemap
