#pragma once

#include <array>

#define MAKE_API_VERSION(major, minor, patch) \
    ((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))

namespace inexor::vulkan_renderer::example_app {

/// The following data will be filled when CMake configures this file
constexpr const char *APP_NAME{"Example-app"};
constexpr std::uint32_t APP_VERSION{MAKE_API_VERSION(0, 1, 0)};
constexpr const char *APP_VERSION_STR{"0.1.0"};
constexpr const char* ENGINE_NAME{"Inexor Engine"};
constexpr std::uint32_t ENGINE_VERSION{MAKE_API_VERSION(0, 1, 0)};
constexpr std::uint32_t USED_VULKAN_API_VERSION{MAKE_API_VERSION(1, 3, 0)};
constexpr const char *ENGINE_VERSION_STR{"0.1.0"};
constexpr const char *BUILD_GIT = "8c7a67f";
constexpr const char *BUILD_TYPE = "Debug";

} // namespace inexor::vulkan_renderer::example_app
