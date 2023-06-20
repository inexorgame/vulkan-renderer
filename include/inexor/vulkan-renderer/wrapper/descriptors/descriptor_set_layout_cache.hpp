#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"

#include <volk.h>

#include <memory>
#include <unordered_map>
#include <vector>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
}

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Forward declaration
class DescriptorBuilder;

///
struct DescriptorSetLayoutInfo {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    [[nodiscard]] bool operator==(const DescriptorSetLayoutInfo &other) const;
    [[nodiscard]] std::size_t hash() const;
};

///
struct DescriptorSetLayoutHash {
    std::size_t operator()(const DescriptorSetLayoutInfo &k) const {
        return k.hash();
    }
};

/// A class for caching VkDescriptorSetLayouts with the help of std::unordered_map and a hashing function
/// For internal use inside of rendergraph only!
class DescriptorSetLayoutCache {
    friend DescriptorBuilder;

private:
    /// The device wrapper
    const Device &m_device;

    /// The actual descriptor set layout cache
    /// Note that std::unordered_map can accept a third template parameter which is the hash function
    /// Also note that the lifetime of the VkDescriptorSetLayout objects is bound to the lifetime of this unordered_map
    /// The destructor of the DescriptorSetLayout wrapper instances will be called when DescriptorSetLayoutCache's
    /// destructor is called
    std::unordered_map<DescriptorSetLayoutInfo, DescriptorSetLayout, DescriptorSetLayoutHash> m_cache;

public:
    /// Default constructor
    /// @param device The device wrapper
    explicit DescriptorSetLayoutCache(const Device &device);

    DescriptorSetLayoutCache(const DescriptorSetLayoutCache &) = delete;
    DescriptorSetLayoutCache(DescriptorSetLayoutCache &&) noexcept;
    ~DescriptorSetLayoutCache() = default;

    DescriptorSetLayoutCache &operator=(const DescriptorSetLayoutCache &) = delete;
    DescriptorSetLayoutCache &operator=(DescriptorSetLayoutCache &&) = delete;

    /// Create a descriptor set layout with the help of the cache
    /// @param descriptor_set_layout_ci The descriptor set layout create info
    /// @return The descriptor set layout that was created
    [[nodiscard]] VkDescriptorSetLayout
    create_descriptor_set_layout(VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci);
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
