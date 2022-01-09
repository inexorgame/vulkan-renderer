#pragma once

#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// forward declarations
class Device;
class ResourceDescriptor;

/// TODO: Implement more descriptor types than just uniform buffers and combined image samplers.
/// TODO: Support uniform buffer offset in VkDescriptorBufferInfo.
class DescriptorBuilder {
private:
    const Device &m_device;
    std::uint32_t m_binding{0};

    std::vector<VkWriteDescriptorSet> m_write_sets;
    std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings;

    std::vector<std::unique_ptr<VkDescriptorBufferInfo>> m_descriptor_buffer_infos;
    std::vector<std::unique_ptr<VkDescriptorImageInfo>> m_descriptor_image_infos;

    // TODO: Summarize pool sizes!
    std::vector<VkDescriptorPoolSize> m_pool_sizes;
    std::unique_ptr<DescriptorPool> m_descriptor_pool;

public:
    /// Construct the descriptor builder
    /// @param device The const reference to a device RAII wrapper instance.
    DescriptorBuilder(const Device &device);

    DescriptorBuilder(const DescriptorBuilder &) = delete;
    DescriptorBuilder(DescriptorBuilder &&) noexcept;
    ~DescriptorBuilder() = default;

    DescriptorBuilder &operator=(const DescriptorBuilder &) = delete;
    DescriptorBuilder &operator=(DescriptorBuilder &&) noexcept = default;

    /// @brief Adds a combined image sampler to the descriptor container.
    /// @param image_sampler The pointer to the combined image sampler
    /// @param image_view The pointer to the image view
    /// @param shader_stage The shader stage the uniform buffer will be used in, most likely the fragment shader
    /// @return A const reference to this DescriptorBuilder instance.
    [[nodiscard]] DescriptorBuilder &
    add_combined_image_sampler(VkSampler image_sampler, VkImageView image_view,
                               VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT);

    /// Adds a combined image sampler to the descriptor container
    /// @param texture The texture.
    /// @return A const reference to this DescriptorBuilder instance.
    [[nodiscard]] DescriptorBuilder &add_combined_image_sampler(const texture::GpuTexture &texture);

    /// Adds a combined image sampler to the descriptor container
    /// @param cubemap The cubemap.
    /// @return A const reference to this DescriptorBuilder instance.
    [[nodiscard]] DescriptorBuilder &add_combined_image_sampler(const cubemap::GpuCubemap &cubemap);

    /// @brief Add combined image samplers for several textures
    /// @param textures The textures.
    /// @return A const reference to this DescriptorBuilder instance.
    [[nodiscard]] DescriptorBuilder &add_combined_image_samplers(const std::vector<texture::GpuTexture> &textures);

    /// @brief Adds a uniform buffer
    /// @tparam T The type of the uniform buffer.
    /// @param uniform_buffer The uniform buffer.
    /// @param shader_stage The shader stage in which the uniform buffer will be used.
    /// @return A const reference to this DescriptorBuilder instance.
    template <typename UniformBufferType>
    [[nodiscard]] DescriptorBuilder &
    add_uniform_buffer(const UniformBuffer<UniformBufferType> &uniform_buffer,
                       const VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_VERTEX_BIT) {

        assert(uniform_buffer.buffer());

        const VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1};
        m_pool_sizes.push_back(pool_size);

        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = m_binding;
        layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layout_binding.descriptorCount = 1;
        layout_binding.stageFlags = shader_stage;

        m_layout_bindings.push_back(std::move(layout_binding));

        auto ub_info = std::make_unique<VkDescriptorBufferInfo>();
        ub_info->buffer = uniform_buffer.buffer();
        ub_info->offset = 0;
        ub_info->range = static_cast<VkDeviceSize>(sizeof(UniformBufferType));

        m_descriptor_buffer_infos.push_back(std::move(ub_info));

        auto desc_write = make_info<VkWriteDescriptorSet>();
        // dstSet will be filled out by the constructor of ResourceDescriptor
        desc_write.dstBinding = m_binding;
        desc_write.dstArrayElement = 0;
        desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        desc_write.descriptorCount = 1;
        desc_write.pBufferInfo = m_descriptor_buffer_infos.back().get();

        m_write_sets.push_back(std::move(desc_write));

        m_binding++;

        return *this;
    }

    /// Build the resource descriptor
    /// @param name The internal name of the resource descriptor.
    /// @return The resource descriptor which was build.
    [[nodiscard]] std::unique_ptr<ResourceDescriptor> build(std::string name);
};

} // namespace inexor::vulkan_renderer::wrapper
