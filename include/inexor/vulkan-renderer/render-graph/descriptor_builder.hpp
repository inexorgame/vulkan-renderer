#pragma once

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <volk.h>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::render_graph {

// Forward declarations
class Device;
class ResourceDescriptor;

/// A builder class for descriptors
class DescriptorBuilder {
    const Device &m_device;

    std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings;
    std::vector<VkWriteDescriptorSet> m_write_sets;
    std::vector<VkDescriptorBufferInfo> m_descriptor_buffer_infos;
    std::vector<VkDescriptorImageInfo> m_descriptor_image_infos;

public:
    /// Constructs the descriptor builder
    /// @param device The device wrapper
    explicit DescriptorBuilder(const Device &device);

    DescriptorBuilder(const DescriptorBuilder &) = delete;
    DescriptorBuilder(DescriptorBuilder &&) = delete;
    ~DescriptorBuilder() = default;

    DescriptorBuilder &operator=(const DescriptorBuilder &) = delete;
    DescriptorBuilder &operator=(DescriptorBuilder &&) = delete;

    // TODO: Implement more descriptor types than just uniform buffers and combined image samplers.
    // TODO: Support uniform buffer offset in VkDescriptorBufferInfo.
    // TODO: Offer overloaded methods which expose more fields of the structures.

    /// Adds a uniform buffer to the descriptor container
    /// @tparam T The type of the uniform buffer
    /// @param uniform_buffer The uniform buffer which contains the data which will be accessed by the shader
    /// @param binding The binding index which will be used in the SPIR-V shader
    /// @param shader_stage The shader stage the uniform buffer will be used in, most likely the vertex shader
    /// @return A const reference to this DescriptorBuilder instance
    template <typename T>
    DescriptorBuilder &add_uniform_buffer(VkBuffer uniform_buffer, std::uint32_t binding,
                                          VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_VERTEX_BIT) {
        assert(uniform_buffer);

        m_layout_bindings.push_back({
            .binding = binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = static_cast<VkShaderStageFlags>(shader_stage),
            .pImmutableSamplers = nullptr,
        });

        m_descriptor_buffer_infos.push_back({
            .buffer = uniform_buffer,
            .offset = 0,
            .range = sizeof(T),
        });

        m_write_sets.push_back(make_info<VkWriteDescriptorSet>({
            .dstSet = nullptr,
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &m_descriptor_buffer_infos.back(),
        }));

        return *this;
    }

    /// Adds a combined image sampler to the descriptor container
    /// @param image_sampler The pointer to the combined image sampler
    /// @param image_view The pointer to the image view
    /// @param binding The binding index which will be used in the SPIR-V shader
    /// @param shader_stage The shader stage the uniform buffer will be used in, most likely the fragment shader
    /// @return A const reference to this DescriptorBuilder instance
    DescriptorBuilder &add_combined_image_sampler(VkSampler image_sampler, VkImageView image_view,
                                                  std::uint32_t binding,
                                                  VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT);

    /// Builds the resource descriptor
    /// @param name The internal name of the resource descriptor
    /// @return The resource descriptor which was created by the builder
    [[nodiscard]] ResourceDescriptor build(std::string name);
};

} // namespace inexor::vulkan_renderer::render_graph
