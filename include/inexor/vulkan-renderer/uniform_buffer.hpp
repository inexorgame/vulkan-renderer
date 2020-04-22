#pragma once

#include "inexor/vulkan-renderer/gpu_memory_buffer.hpp"

namespace inexor::vulkan_renderer {

///
///
///
struct InexorUniformBuffer : public InexorBuffer {
    //
    VkDescriptorBufferInfo descriptor_buffer_info;

    //
    VkDescriptorSet descriptor_set;
};

} // namespace inexor::vulkan_renderer
