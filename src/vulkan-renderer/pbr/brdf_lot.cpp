#include "inexor/vulkan-renderer/pbr/brdf_lut.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"

#include <spdlog/spdlog.h>

#include <vector>

namespace inexor::vulkan_renderer {

BrdfLutGenerator::BrdfLutGenerator(const VkDevice device, const std::array<wrapper::Shader, 2> &shaders) {
    spdlog::trace("BRDF LUT generation finished");
}

} // namespace inexor::vulkan_renderer
