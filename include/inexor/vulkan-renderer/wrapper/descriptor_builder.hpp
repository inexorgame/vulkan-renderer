#pragma once

#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// forward declarations
class Device;
class ResourceDescriptor;

class DescriptorBuilder {
private:
    const Device &m_device;
    std::uint32_t m_binding{0};

    std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings;
    std::vector<VkWriteDescriptorSet> m_write_sets;
    std::vector<VkDescriptorBufferInfo> m_descriptor_buffer_infos;
    std::vector<VkDescriptorImageInfo> m_descriptor_image_infos;
    VkDescriptorPool m_descriptor_pool;

public:
    /// @brief Constructs the descriptor builder.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param descriptor_pool The descriptor pool.
    DescriptorBuilder(const Device &device, VkDescriptorPool descriptor_pool);

    DescriptorBuilder(DescriptorBuilder &&) noexcept;
    ~DescriptorBuilder() = default;

    DescriptorBuilder &operator=(const DescriptorBuilder &) = delete;
    DescriptorBuilder &operator=(DescriptorBuilder &&) = delete;

    // TODO: Implement more descriptor types than just uniform buffers and combined image samplers.
    // TODO: Support uniform buffer offset in VkDescriptorBufferInfo.
    // TODO: Offer overloaded methods which expose more fields of the structures.

    /// @brief Adds a uniform buffer to the descriptor container.
    /// @tparam T The type of the uniform buffer
    /// @param uniform_buffer The uniform buffer which contains the data which will be accessed by the shader
    /// @param shader_stage The shader stage the uniform buffer will be used in, most likely the vertex shader
    /// @return A const reference to this DescriptorBuilder instance
    template <typename T>
    [[nodiscard]] DescriptorBuilder &
    add_uniform_buffer(VkBuffer uniform_buffer, VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_VERTEX_BIT);

    /// @brief Adds a combined image sampler to the descriptor container.
    /// @param image_sampler The pointer to the combined image sampler
    /// @param image_view The pointer to the image view
    /// @param shader_stage The shader stage the uniform buffer will be used in, most likely the fragment shader
    /// @return A const reference to this DescriptorBuilder instance
    [[nodiscard]] DescriptorBuilder &
    add_combined_image_sampler(VkSampler image_sampler, VkImageView image_view,
                               VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT);

    /// @brief Adds a combined image sampler to the descriptor container.
    /// @param image_sampler The pointer to the combined image sampler
    /// @return A const reference to this DescriptorBuilder instance
    [[nodiscard]] DescriptorBuilder &add_combined_image_sampler(const texture::GpuTexture &texture);

    DescriptorBuilder &DescriptorBuilder::add_combined_image_sampler(const cubemap::GpuCubemap &cubemap);

    /// @brief Add combined image samplers for every given texture
    /// @param textures The textures
    /// @return A const reference to this DescriptorBuilder instance
    [[nodiscard]] DescriptorBuilder &add_combined_image_samplers(const std::vector<texture::GpuTexture> &textures);

    /// @brief Builds the resource descriptor.
    /// @param name The internal name of the resource descriptor.
    /// @return The resource descriptor which was created by the builder.
    [[nodiscard]] std::unique_ptr<ResourceDescriptor> build(std::string name);
};

template <typename T>
DescriptorBuilder &DescriptorBuilder::add_uniform_buffer(const VkBuffer uniform_buffer,
                                                         const VkShaderStageFlagBits shader_stage) {
    assert(uniform_buffer);

    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = m_binding;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = shader_stage;
    layout_binding.pImmutableSamplers = nullptr;

    m_layout_bindings.push_back(layout_binding);

    VkDescriptorBufferInfo uniform_buffer_info{};
    uniform_buffer_info.buffer = uniform_buffer;
    uniform_buffer_info.offset = 0;
    uniform_buffer_info.range = sizeof(T);

    m_descriptor_buffer_infos.push_back(uniform_buffer_info);

    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = nullptr;
    descriptor_write.dstBinding = m_binding;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &m_descriptor_buffer_infos.back();

    m_write_sets.push_back(descriptor_write);

    // m_binding++;

    return *this;
}

} // namespace inexor::vulkan_renderer::wrapper
