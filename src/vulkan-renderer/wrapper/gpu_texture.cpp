#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

GpuTexture::GpuTexture(const Device &device, const CpuTexture &cpu_texture) : TextureBase(device) {
    m_attributes = cpu_texture.attributes();
    create_image(cpu_texture.data(), cpu_texture.data_size());
    create_default_texture_sampler();
}

GpuTexture::GpuTexture(const Device &device, const void *data, std::size_t data_size, const std::uint32_t width,
                       const std::uint32_t height, const std::uint32_t channel_count,
                       const std::uint32_t miplevel_count, std::string name)
    : TextureBase(device) {

    assert(data);
    assert(!name.empty());

    m_attributes.width = width;
    m_attributes.height = height;
    m_attributes.mip_levels = miplevel_count;
    m_attributes.channels = channel_count;
    m_attributes.name = name;

    create_image(data, data_size);
    create_default_texture_sampler();
}

GpuTexture::GpuTexture(const Device &device, const gltf::TextureSampler &sampler, const CpuTexture &cpu_texture)
    : TextureBase(device) {
    m_attributes = cpu_texture.attributes();
    create_image(cpu_texture.data(), cpu_texture.data_size());
    create_texture_sampler(sampler);
}

GpuTexture::GpuTexture(const Device &device, const gltf::TextureSampler &sampler, const void *data,
                       std::size_t data_size, std::uint32_t width, std::uint32_t height, std::uint32_t channel_count,
                       std::uint32_t miplevel_count, std::string name)
    : TextureBase(device) {

    assert(data);
    assert(!name.empty());

    m_attributes.width = width;
    m_attributes.height = height;
    m_attributes.mip_levels = miplevel_count;
    m_attributes.channels = channel_count;
    m_attributes.name = name;

    create_image(data, data_size);
    create_texture_sampler(sampler);
}

// TODO: Is this technically correct?
GpuTexture::GpuTexture(GpuTexture &&other) noexcept : TextureBase(other.m_device) {}

GpuTexture::~GpuTexture() {
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
}

// TODO: We can just get that default setting and remove glTF dependency from gpu_texture here!
void GpuTexture::create_texture_sampler(const gltf::TextureSampler &sampler) {
    // We just copy the default settings and only overwrite those specified by gltf sampler
    auto sampler_ci = wrapper::default_sampler_settings(m_device.physical_device());

    sampler_ci.magFilter = sampler.mag_filter();
    sampler_ci.minFilter = sampler.min_filter();
    sampler_ci.addressModeU = sampler.address_mode_u();
    sampler_ci.addressModeV = sampler.address_mode_v();
    sampler_ci.addressModeW = sampler.address_mode_w();

    TextureBase::create_texture_sampler(sampler_ci);
}

} // namespace inexor::vulkan_renderer::wrapper
