#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;
class ResourceDescriptor;

class DescriptorBuilder {
    const Device &m_device;
    const std::uint32_t m_swapchain_image_count{};

    std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings{};
    std::vector<VkWriteDescriptorSet> m_write_sets{};
    std::vector<VkDescriptorType> m_descriptor_types{};
    std::vector<VkDescriptorBufferInfo> m_descriptor_buffer_infos{};

public:
    /// @brief
    /// @param device
    /// @param swapchain_image_count
    DescriptorBuilder(const Device &device, const std::uint32_t swapchain_image_count);

    DescriptorBuilder(const DescriptorBuilder &) = delete;
    DescriptorBuilder(DescriptorBuilder &&) noexcept;
    ~DescriptorBuilder() = default;

    DescriptorBuilder &operator=(const DescriptorBuilder &) = delete;
    DescriptorBuilder &operator=(DescriptorBuilder &&) = delete;

    // TODO: Implement more descriptor types than just uniform buffers here!
    // TODO: Support more than 1 descriptor per descriptor set.
    // TODO: Support uniform buffer offset in VkDescriptorBufferInfo.

    /// @brief Adds a uniform buffer to the descriptor container.
    /// @tparam T The type of the uniform buffer.
    /// @param uniform_buffer The uniform buffer which contains the data which will be accessed by the shader.
    /// @param binding The binding index which will be used in the SPIR-V shader.
    /// @param shader_stage The shader stage the uniform buffer will be used in, most likely the vertex shader.
    /// @return A const reference to this DescriptorBuilder instance.
    template <typename T>
    DescriptorBuilder &add_uniform_buffer(const VkBuffer uniform_buffer, const std::uint32_t binding,
                                          const VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_VERTEX_BIT) {
        assert(uniform_buffer);

        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = binding;
        layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layout_binding.descriptorCount = 1;
        layout_binding.stageFlags = shader_stage;
        layout_binding.pImmutableSamplers = nullptr;

        m_layout_bindings.push_back(layout_binding);

        m_descriptor_types.emplace_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

        VkDescriptorBufferInfo uniform_buffer_info{};
        uniform_buffer_info.buffer = uniform_buffer;
        uniform_buffer_info.offset = 0;
        uniform_buffer_info.range = sizeof(T);

        m_descriptor_buffer_infos.push_back(uniform_buffer_info);

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = nullptr;
        descriptor_write.dstBinding = binding;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &m_descriptor_buffer_infos.back();

        m_write_sets.push_back(descriptor_write);

        return *this;
    }

    /// @brief
    /// @param name
    /// @return
    ResourceDescriptor build(std::string name);
};

}; // namespace inexor::vulkan_renderer::wrapper
