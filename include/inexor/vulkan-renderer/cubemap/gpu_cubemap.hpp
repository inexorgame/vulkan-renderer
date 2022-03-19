#pragma once

#include "inexor/vulkan-renderer/texture/cpu_texture.hpp"
#include "inexor/vulkan-renderer/texture/sampler.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::cubemap {

class GpuCubemap : public wrapper::Image {
private:
    const wrapper::Device &m_device;

    static constexpr std::uint32_t FACE_COUNT{6};

    std::unique_ptr<texture::Sampler> m_sampler;

    VkImageCreateInfo m_image_ci;
    VkImageViewCreateInfo m_image_view_ci;
    VkSamplerCreateInfo m_sampler_ci;

    std::string m_name;

    [[nodiscard]] VkImageCreateInfo fill_image_ci(VkFormat format, std::uint32_t width, std::uint32_t height,
                                                  std::uint32_t miplevel_count);

    [[nodiscard]] VkImageCreateInfo fill_image_ci(VkFormat format, const texture::CpuTexture &cpu_cubemap);

    [[nodiscard]] VkImageViewCreateInfo fill_image_view_ci(VkFormat format, std::uint32_t miplevel_count);

    [[nodiscard]] VkSamplerCreateInfo fill_sampler_ci(std::uint32_t miplevel_count);

public:
    GpuCubemap(const wrapper::Device &device, VkImageCreateInfo m_image_ci, VkImageViewCreateInfo m_image_view_ci,
               VkSamplerCreateInfo m_sampler_ci, std::string name);

    GpuCubemap(const wrapper::Device &device, VkFormat format, const texture::CpuTexture &cpu_cubemap,
               std::string name);

    GpuCubemap(const wrapper::Device &device, const texture::CpuTexture &cpu_cubemap);

    GpuCubemap(const wrapper::Device &device, VkFormat format, std::uint32_t width, std::uint32_t height,
               std::uint32_t miplevel_count, std::string name);

    GpuCubemap(const wrapper::Device &device, VkFormat format, std::uint32_t width, std::uint32_t height,
               std::string name);

    void copy_from_image(const wrapper::CommandBuffer &cmd_buf, VkImage source_image, std::uint32_t face,
                         std::uint32_t mip_level, std::uint32_t width, std::uint32_t height);

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

    [[nodiscard]] VkSampler sampler() const {
        return m_sampler->sampler();
    }
};

} // namespace inexor::vulkan_renderer::cubemap
