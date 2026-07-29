#pragma once

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"

#include <volk.h>

#include <string>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Using declaration
using tools::InexorException;

/// A simplified helper enum for descriptor types
enum class DescriptorType {
    SAMPLER,
    COMBINED_IMAGE_SAMPLER,
    SAMPLED_IMAGE,
    STORAGE_IMAGE,
    UNIFORM_TEXEL_BUFFER,
    STORAGE_TEXEL_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
    UNIFORM_BUFFER_DYNAMIC,
    STORAGE_BUFFER_DYNAMIC,
    INPUT_ATTACHMENT
};

/// A builder for descriptors
class DescriptorSetLayoutBuilder {
private:
    const Device &m_device;
    /// All instances of DescriptorSetLayoutBuilder have the same DescriptorSetLayoutCache instance!
    DescriptorSetLayoutCache m_descriptor_set_layout_cache;
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;
    std::uint32_t m_binding = 0;

public:
    /// Default constructor
    /// @param device The device wrapper
    DescriptorSetLayoutBuilder(const Device &device);

    /// Add a new descriptor
    /// @param type The type of the descriptor
    /// @param stage The shader stage
    /// @param count The number of descriptors
    /// @note It was deliberately decided not to guess the shader stage based on DescriptorType, simply because one
    /// descriptor type could be used in various shader stages. Another approach would have been to make the shader
    /// stage an std::optional, and if it's specified as std::nullopt, decide which shader stage is most likely based on
    /// the DescriptorType. However, this approach is not clearly defined, because one descriptor could be used in a
    /// variety of shader stages.
    [[nodiscard]] auto &add(DescriptorType type, VkShaderStageFlags stage, std::uint32_t count = 1) {
        m_bindings.emplace_back(VkDescriptorSetLayoutBinding{
            .binding = m_binding++, // Use current binding, then increment
            .descriptorType =
                [&]() {
                    switch (type) {
                    case DescriptorType::SAMPLER:
                        return VK_DESCRIPTOR_TYPE_SAMPLER;
                    case DescriptorType::COMBINED_IMAGE_SAMPLER:
                        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    case DescriptorType::SAMPLED_IMAGE:
                        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    case DescriptorType::STORAGE_IMAGE:
                        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    case DescriptorType::UNIFORM_TEXEL_BUFFER:
                        return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                    case DescriptorType::STORAGE_TEXEL_BUFFER:
                        return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                    case DescriptorType::UNIFORM_BUFFER:
                        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    case DescriptorType::STORAGE_BUFFER:
                        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
                        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    case DescriptorType::STORAGE_BUFFER_DYNAMIC:
                        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    case DescriptorType::INPUT_ATTACHMENT:
                        return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                    default:
                        throw InexorException("Unknown DescriptorType!");
                    }
                }(),
            .descriptorCount = count,
            .stageFlags = stage,
        });
        return *this;
    }

    // @TODO Enforce a return type for the build() method so the rendergraph only accepts fully built descriptor set
    // layouts, avoiding the descriptor set layout builder to be in incomplete, invalid state!

    /// Build the descriptor set layout
    /// @param name The name of the descriptor set layout
    /// @return The descriptor set layout that was created
    [[nodiscard]] VkDescriptorSetLayout build(std::string name);
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
