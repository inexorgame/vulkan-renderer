#pragma once

#include "inexor/vulkan-renderer/tools/file.hpp"

#include <vulkan/vulkan.h>

#include <string>

namespace inexor::vulkan_renderer {

class Shader : public tools::File {
public:
    Shader() = default;

    ~Shader() = default;

    std::string name;

    std::string entry_name;

    VkShaderStageFlagBits type;

    VkShaderModule module;
};

} // namespace inexor::vulkan_renderer
