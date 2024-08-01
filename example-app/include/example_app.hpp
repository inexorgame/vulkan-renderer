#pragma once

#include "../include/example_app_base.hpp"

#include "inexor/vulkan-renderer/rendering/imgui/imgui.hpp"
#include "inexor/vulkan-renderer/rendering/octree/octree_renderer.hpp"

#include <spdlog/spdlog.h>

namespace inexor::example_app {

/// An example application using Inexor vulkan-renderer
class ExampleApp : public ExampleAppBase {
private:
    std::unique_ptr<ImGuiRenderer> m_imgui_renderer;
    std::unique_ptr<OctreeRenderer> m_octree_renderer;

public:
    ExampleApp(int argc, char **argv);
    ~ExampleApp();

    ExampleApp(const ExampleApp &) = delete;
    // TODO: Implement me!
    ExampleApp(ExampleApp &&) noexcept;

    ExampleApp &operator=(const ExampleApp &) = delete;
    // TODO: Implement me!
    ExampleApp &operator=(ExampleApp &&) noexcept;

    void initialize() override;

    void load_toml_configuration_file(const std::string &file_name);

    void setup_render_graph() override;

    void cursor_position_callback(GLFWwindow *window, double x_pos, double y_pos) override;

    void keyboard_button_callback(GLFWwindow *window, int key, int scancode, int action, int mods) override;

    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) override;

    void mouse_scroll_callback(GLFWwindow *window, double x_offset, double y_offset) override;

    void update_imgui() override;
};

} // namespace inexor::example_app
