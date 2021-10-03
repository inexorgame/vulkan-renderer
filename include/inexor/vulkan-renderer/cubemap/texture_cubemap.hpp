#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"

namespace inexor::vulkan_renderer::cubemap {

/// @brief
class TextureCubeMap : public wrapper::GpuTexture {

public:
    TextureCubeMap();
};

} // namespace inexor::vulkan_renderer::cubemap
