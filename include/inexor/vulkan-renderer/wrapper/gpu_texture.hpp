#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_texture_sampler.hpp"
#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/texture_attributes.hpp"

#include <vulkan/vulkan.h>

#include <ktxvulkan.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// TODO: Support texture arrays!

/// @note The code which loads textures from files is wrapped in CpuTexture.
/// @brief RAII wrapper class for textures which are stored in GPU memory.
class GpuTexture {
    const Device &m_device;
    TextureAttributes m_attributes;
    std::unique_ptr<Image> m_texture_image;

    VkSampler m_sampler{VK_NULL_HANDLE};
    VkDescriptorImageInfo m_descriptor;

    void create_image(const void *texture_data, std::size_t texture_size);

    void create_cubemap_image(const void *texture_data, std::size_t texture_size, ktxTexture* texture);

    /// @brief Transform the image layout.
    /// @param image The image
    /// @param old_layout The old image layout
    /// @param new_layout The new image layout
    void transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);

    void create_default_texture_sampler();

    void create_texture_sampler(VkSamplerCreateInfo sampler_ci);

    void create_texture_sampler(const gltf::TextureSampler &sampler);

    void update_descriptor();

public:
    GpuTexture(const Device &device, const CpuTexture &cpu_texture);

    GpuTexture(const Device &device, const CpuTexture &cpu_texture, std::uint32_t faces);

    // TODO: Support mip levels here!
    GpuTexture(const Device &device, const void *data, std::size_t data_size, std::uint32_t width, std::uint32_t height,
               std::uint32_t channel_count, std::uint32_t miplevel_count, std::string name);

    GpuTexture(const Device &device, const gltf::TextureSampler &sampler, const CpuTexture &cpu_texture);

    GpuTexture(const Device &device, const gltf::TextureSampler &sampler, const void *data, std::size_t data_size,
               std::uint32_t width, std::uint32_t height, std::uint32_t channel_count, std::uint32_t miplevel_count,
               std::string name);

    GpuTexture(const GpuTexture &) = delete;
    GpuTexture(GpuTexture &&) noexcept;
    ~GpuTexture();

    GpuTexture &operator=(const GpuTexture &) = delete;
    GpuTexture &operator=(GpuTexture &&) = delete;

    [[nodiscard]] VkImage image() const {
        return m_texture_image->image();
    }

    [[nodiscard]] auto &image_wrapper() const {
        return m_texture_image;
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_texture_image->image_view();
    }

    [[nodiscard]] VkSampler sampler() const {
        return m_sampler;
    }

    [[nodiscard]] auto &descriptor() const {
        return m_descriptor;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
