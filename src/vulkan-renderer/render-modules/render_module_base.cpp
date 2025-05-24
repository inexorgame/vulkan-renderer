#include "inexor/vulkan-renderer/render-modules/render_module_base.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/exception.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::render_modules {

RenderModuleBase::RenderModuleBase(const Device &device, std::string name)
    : m_device(device), m_name(std::move(name)) {}

} // namespace inexor::vulkan_renderer::render_modules
