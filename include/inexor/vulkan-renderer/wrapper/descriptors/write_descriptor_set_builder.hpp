#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <volk.h>

#include <memory>
#include <variant>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class Buffer;
enum class BufferType;
class Texture;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Using declarations
using render_graph::Buffer;
using render_graph::BufferType;
using render_graph::Texture;
using tools::InexorException;

/// A wrapper class for batching calls to vkUpdateDescriptorSets
class WriteDescriptorSetBuilder {
private:
    const Device &m_device;
    std::vector<VkWriteDescriptorSet> m_write_descriptor_sets;
    std::uint32_t m_binding{0};

    /// Reset the data of the builder so it can be re-used
    void reset();

public:
    /// Default constructor
    /// @param device The device wrapper
    explicit WriteDescriptorSetBuilder(const Device &device);

    /// Add a new entry to the write descriptor set builder
    /// @param descriptor_set The descriptor set
    /// @param descriptor_data Either a buffer or a texture
    /// @param descriptor_count The descriptor count (``1`` by default)
    [[nodiscard]] auto add(const VkDescriptorSet descriptor_set,
                           std::variant<std::weak_ptr<Texture>, std::weak_ptr<Buffer>> descriptor_data,
                           std::uint32_t descriptor_count = 1) {
        if (!descriptor_set) {
            throw InexorException("Error: Parameter 'descriptor_set' is invalid!");
        }

        // @TODO How would descriptor_count greater than 1 be implemented here?

        auto write_descriptor_set = wrapper::make_info<VkWriteDescriptorSet>({
            .dstSet = descriptor_set,
            .dstBinding = m_binding,
            .dstArrayElement = 0,
            .descriptorCount = descriptor_count,
        });

        // This short code is presented to you by the magic of variant-based polymorphism :)
        // Handle the variant type (either Texture or Buffer)
        std::visit(
            [&write_descriptor_set](auto &&descriptor) {
                using T = std::decay_t<decltype(descriptor)>;
                if constexpr (std::is_same_v<T, std::weak_ptr<Texture>>) {
                    if (auto texture = descriptor.lock(); texture) {
                        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write_descriptor_set.pImageInfo = texture->descriptor_image_info();
                    } else {
                        throw InexorException("Error: Texture is invalid!");
                    }
                } else if constexpr (std::is_same_v<T, std::weak_ptr<Buffer>>) {
                    if (auto buffer = descriptor.lock(); buffer) {
                        // TODO: Distinguish type by buffer->type()! (uniform ? storage? ..)
                        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        write_descriptor_set.pBufferInfo = buffer->descriptor_buffer_info();
                    } else {
                        throw InexorException("Error: Buffer is invalid!");
                    }
                } else {
                    // TODO: Support more descriptor types
                    throw InexorException("Error: Invalid descriptor type in std::variant!");
                }
            },
            descriptor_data);

        m_write_descriptor_sets.push_back(write_descriptor_set);
        m_binding++;
        return *this;
    }

    /// Return the write descriptor sets and reset the builder
    /// @return A std::vector of VkWriteDescriptorSet
    [[nodiscard]] std::vector<VkWriteDescriptorSet> build();
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
