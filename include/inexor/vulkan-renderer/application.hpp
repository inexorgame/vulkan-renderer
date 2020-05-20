﻿#pragma once

#include "inexor/vulkan-renderer/renderer.hpp"
#include "inexor/vulkan-renderer/thread_pool.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer {

class Application : public VulkanRenderer {
public:
    Application() = default;

    ~Application() = default;

private:
    std::string application_name = "";

    std::string engine_name = "";

    std::uint32_t application_version = 0;

    std::uint32_t engine_version = 0;

    // The core concept of paralellization in Inexor is to use a
    // C++17 threadpool implementation which spawns worker threads.
    // A task system is used to distribute work over worker threads.
    // Call thread_pool->execute(); to order new tasks to be worked on.
    std::shared_ptr<ThreadPool> thread_pool;

    std::size_t current_frame = 0;

    // TODO: Refactor into a manger class.
    struct ShaderSetup {
        VkShaderStageFlagBits shader_type;
        std::string shader_file_name;
    };

    std::vector<std::string> vertex_shader_files;

    std::vector<std::string> fragment_shader_files;

    std::vector<std::string> texture_files;

    std::vector<std::string> shader_files;

    std::vector<std::string> gltf_model_files;

private:
    /// @brief Loads the configuration of the renderer from a TOML configuration file.
    /// @brief file_name [in] The TOML configuration file.
    /// @note It was collectively decided not to use JSON for configuration files.
    VkResult load_toml_configuration_file(const std::string &file_name);

    VkResult load_textures();

    VkResult load_shaders();

    VkResult load_models();

    VkResult load_octree_geometry();

    VkResult check_application_specific_features();

    VkResult render_frame();

    /// @brief Implementation of the uniform buffer update method.
    /// @param current_image [in] The current image index.
    VkResult update_uniform_buffers(const std::size_t current_image);

    VkResult update_keyboard_input();

    VkResult update_mouse_input();

    // TODO: Refactor!
    double cursor_x, cursor_y;

public:
    VkResult init(int argc, char **argv);

    /// @brief Keyboard input callback.
    /// @param window [in] The glfw window.
    /// @param key [in] The key which was pressed or released.
    /// @param scan_code [in] The system-specific scancode of the key.
    /// @param action [in] The key action: GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
    /// @param mods [in] Bit field describing which modifier keys were held down.
    void keyboard_input_callback(GLFWwindow *window, int key, int scan_code, int action, int mods);

    void run();

    void cleanup();
};

} // namespace inexor::vulkan_renderer
