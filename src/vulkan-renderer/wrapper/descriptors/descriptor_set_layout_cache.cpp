#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <algorithm>
#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorSetLayoutCache::DescriptorSetLayoutCache(const Device &device) : m_device(device) {}

DescriptorSetLayoutCache::DescriptorSetLayoutCache(DescriptorSetLayoutCache &&other) noexcept
    : m_device(other.m_device) {
    m_cache = std::move(other.m_cache);
}

VkDescriptorSetLayout
DescriptorSetLayoutCache::create_descriptor_set_layout(const VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci,
                                                       std::string name) {
    DescriptorSetLayoutInfo layout_info;
    layout_info.bindings.reserve(descriptor_set_layout_ci.bindingCount);
    bool is_sorted = true;
    int last_binding = -1;

    // Loop through all bindings and ensure that the bindings are in increasing order
    for (std::size_t i = 0; i < descriptor_set_layout_ci.bindingCount; i++) {
        // Copy the bindings into layout_info
        layout_info.bindings.push_back(descriptor_set_layout_ci.pBindings[i]);

        // Check if the descriptor set layout bindings are sorted by binding
        if (descriptor_set_layout_ci.pBindings[i].binding < last_binding) {
            last_binding = descriptor_set_layout_ci.pBindings[i].binding;
        } else {
            is_sorted = false;
            // As soon as we know it's not sorted, we can stop and start sorting
            break;
        }
    }
    // We need to make sure the bindings are sorted because this is important for the hash!
    if (!is_sorted) {
        std::sort(layout_info.bindings.begin(), layout_info.bindings.end(), [](auto &a, auto &b) {
            return a.binding < b.binding; // Sort by binding
        });
    }

    // Check if this descriptor set layout does already exist in the cache
    if (!m_cache.contains(layout_info)) {
        m_cache.emplace(layout_info, DescriptorSetLayout(m_device, descriptor_set_layout_ci, std::move(name)));
    }
    // TODO: Name descriptor set layout internally!
    return m_cache.at(layout_info).m_descriptor_set_layout;
}

bool DescriptorSetLayoutInfo::operator==(const DescriptorSetLayoutInfo &other) const {
    if (other.bindings.size() != bindings.size()) {
        return false;
    }
    // Check if each of the bindings is the same
    // Note that we assume the bindings are sorted!
    for (std::size_t i = 0; i < bindings.size(); i++) {
        if (other.bindings[i].binding != bindings[i].binding) {
            return false;
        }
        if (other.bindings[i].descriptorType != bindings[i].descriptorType) {
            return false;
        }
        if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) {
            return false;
        }
        if (other.bindings[i].stageFlags != bindings[i].stageFlags) {
            return false;
        }
    }
    return true;
}

std::size_t DescriptorSetLayoutInfo::hash() const {
    assert(!bindings.empty());
    std::size_t result = std::hash<std::size_t>()(bindings.size());
    for (const auto &binding : bindings) {
        // Pack binding data into 64 bits
        std::size_t binding_hash =
            binding.binding | binding.descriptorType << 8 | binding.descriptorCount << 16 | binding.stageFlags << 24;
        // shuffle the packed binding data and xor it with the main hash
        result ^= std::hash<std::size_t>()(binding_hash);
    }
    return result;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
