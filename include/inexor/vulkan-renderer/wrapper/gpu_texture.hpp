#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_texture_sampler.hpp"
#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/texture_base.hpp"

#include <vulkan/vulkan.h>

#include <ktxvulkan.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

/// TODO: Support texture arrays
class GpuTexture : public TextureBase {
private:
    void create_texture_sampler(const gltf::TextureSampler &sampler);

public:
    GpuTexture(const Device &device, const CpuTexture &cpu_texture);

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
};

} // namespace inexor::vulkan_renderer::wrapper
