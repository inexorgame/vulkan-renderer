#pragma once

#include "inexor/vulkan-renderer/texture/cpu_texture.hpp"
#include "inexor/vulkan-renderer/texture/sampler.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <ktxvulkan.h>
#include <vulkan/vulkan.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::texture {

class GpuTexture {
private:
    const wrapper::Device &m_device;

    std::unique_ptr<Sampler> m_sampler;

    VkImageCreateInfo m_image_ci;
    VkImageViewCreateInfo m_image_view_ci;
    VkSamplerCreateInfo m_sampler_ci;

    std::string m_name;

    static constexpr VkFormat DEFAULT_FORMAT{VK_FORMAT_R8G8B8A8_UNORM};

    [[nodiscard]] VkImageCreateInfo make_image_ci(VkFormat format, std::uint32_t width, std::uint32_t height);

    [[nodiscard]] VkImageViewCreateInfo make_image_view_ci(VkFormat format);

    [[nodiscard]] VkSamplerCreateInfo make_sampler_ci(const wrapper::Device &device);

    void upload_texture_data(const void *texture_data, std::size_t texture_size);

public:
    std::unique_ptr<wrapper::Image> m_image;

    GpuTexture(const wrapper::Device &device, const void *texture_data, std::size_t texture_size,
               VkImageCreateInfo image_ci, VkImageViewCreateInfo image_view_ci, VkSamplerCreateInfo sampler_ci,
               std::string name);

    GpuTexture(const wrapper::Device &device, VkImageCreateInfo image_ci, VkImageViewCreateInfo image_view_ci,
               VkSamplerCreateInfo sampler_ci, std::string name);

    GpuTexture(const wrapper::Device &device, const CpuTexture &cpu_texture, VkImageCreateInfo image_ci,
               VkImageViewCreateInfo image_view_ci, VkSamplerCreateInfo sampler_ci);

    GpuTexture(const wrapper::Device &device, VkFormat format, const CpuTexture &cpu_texture);

    GpuTexture(const wrapper::Device &device, const CpuTexture &cpu_texture);

    GpuTexture(const wrapper::Device &device);

    GpuTexture(const GpuTexture &) = delete;
    GpuTexture(GpuTexture &&other) noexcept;

    GpuTexture &operator=(const GpuTexture &) = delete;
    GpuTexture &operator=(GpuTexture &&) noexcept = default;

    [[nodiscard]] VkSampler sampler() const {
        return m_sampler->sampler();
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_image->image_view();
    }

    [[nodiscard]] VkFormat format() const {
        return m_image->format();
    }
};

} // namespace inexor::vulkan_renderer::texture
