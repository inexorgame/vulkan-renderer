#include "../include/example_app.hpp"

#include "../include/example_app_meta.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

namespace inexor::vulkan_renderer::example_app {

// Using declarations
using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::WriteDescriptorSetBuilder;

ExampleApp::ExampleApp(int argc, char *argv[]) {
    parse_command_line_arguments(argc, argv);

    initialize_spdlog();
    spdlog::trace("{}, VERSION: {}.{}.{}", ENGINE_NAME, VK_VERSION_MAJOR(ENGINE_VERSION),
                  VK_VERSION_MINOR(ENGINE_VERSION), VK_VERSION_PATCH(ENGINE_VERSION));
    spdlog::trace("{}, VERSION: {}.{}.{}, BUILD: {} {}, GIT-SHA: {}", APP_NAME, VK_VERSION_MAJOR(APP_VERSION),
                  VK_VERSION_MINOR(APP_VERSION), VK_VERSION_PATCH(APP_VERSION), __DATE__, __TIME__, BUILD_GIT);

    m_window = std::make_unique<Window>(APP_NAME, 1920, 1080, true, true, Window::Mode::WINDOWED);
    setup_window_input_callbacks();

    m_instance = std::make_unique<Instance>(USED_VULKAN_API_VERSION, APP_NAME, ENGINE_NAME, APP_VERSION, ENGINE_VERSION,
                                            validation_layer_debug_messenger_callback);

    m_surface = std::make_unique<Surface>(*m_instance, *m_window);

    create_physical_device();

    m_swapchain = std::make_unique<Swapchain>(*m_device, APP_NAME, *m_surface, *m_window, m_options.vsync_enabled);

    setup_render_graph();
    setup_octree_rendering();
    recreate_swapchain();
}

void ExampleApp::process_mouse_input() {
    const auto cursor_pos_delta = m_input_data.calculate_cursor_position_delta();

    if (m_camera->type() == CameraType::LOOK_AT && m_input_data.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_camera->rotate(static_cast<float>(cursor_pos_delta[0]), -static_cast<float>(cursor_pos_delta[1]));
    }

    m_camera->set_movement_state(CameraMovement::FORWARD, m_input_data.is_key_pressed(GLFW_KEY_W));
    m_camera->set_movement_state(CameraMovement::LEFT, m_input_data.is_key_pressed(GLFW_KEY_A));
    m_camera->set_movement_state(CameraMovement::BACKWARD, m_input_data.is_key_pressed(GLFW_KEY_S));
    m_camera->set_movement_state(CameraMovement::RIGHT, m_input_data.is_key_pressed(GLFW_KEY_D));
}

void ExampleApp::process_keyboard_input() {}

void ExampleApp::create_physical_device() {
    std::vector<const char *> required_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    const VkPhysicalDeviceFeatures required_features = {};

    // TODO: Respect gpu device selection again!

    const auto selected_gpu =
        Device::pick_best_physical_device(*m_instance, *m_surface, required_features, required_extensions);

    m_device = std::make_unique<Device>(*m_instance, *m_surface, selected_gpu, required_extensions, required_features);
}

void ExampleApp::setup_window_input_callbacks() {
    m_window->set_user_ptr(this);

    m_window->set_resize_callback([](GLFWwindow *window, int width, int height) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->m_window->announce_resize();
    });
    m_window->set_keyboard_button_callback([](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->keyboard_button_callback(window, key, scancode, action, mods);
    });
    m_window->set_cursor_position_callback([](GLFWwindow *window, double xpos, double ypos) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->cursor_position_callback(window, xpos, ypos);
    });
    m_window->set_mouse_button_callback([](GLFWwindow *window, int button, int action, int mods) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->mouse_button_callback(window, button, action, mods);
    });
    m_window->set_mouse_scroll_callback([](GLFWwindow *window, double xoffset, double yoffset) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->mouse_scroll_callback(window, xoffset, yoffset);
    });
}

void ExampleApp::initialize_spdlog() {
    spdlog::init_thread_pool(8192, 2);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // A copy of the console output will be saved to a logfile
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string(APP_NAME) + ".log", true);
    auto logger = std::make_shared<spdlog::async_logger>("main", spdlog::sinks_init_list{console_sink, file_sink},
                                                         spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->flush_on(spdlog::level::trace);
    logger->set_level(spdlog::level::trace);
    logger->set_pattern("%Y-%m-%d %T.%f %^%l%$ %5t [%n] %v");
    spdlog::set_default_logger(logger);
}

void ExampleApp::parse_command_line_arguments(int argc, char *argv[]) {
    CommandLineArgumentParser cla_parser;
    cla_parser.parse_args(argc, argv);
    m_options.vsync_enabled = cla_parser.arg<bool>("--vsync").value_or(false);
    m_options.stop_on_validation_error = cla_parser.arg<bool>("--stop-on-validation-error").value_or(false);
    m_options.preferred_gpu = cla_parser.arg<std::uint32_t>("--gpu");
}

void ExampleApp::recreate_swapchain() {
    m_window->wait_for_focus();
    m_device->wait_idle();

    const auto window_size = m_window->get_framebuffer_size();
    m_swapchain->setup(window_size.first, window_size.second, m_options.vsync_enabled);

    // TODO: Do we really need to recreate ImGui overlay?

    // TODO: Aren't we "recompiling" rendergraph on every frame anyways?
    // Split "compile" into the things we really need to do on a per-frame basis
    m_rendergraph->compile();
}

void ExampleApp::run() {
    while (!m_window->should_close()) {
        m_window->poll();
        process_mouse_input();
        process_keyboard_input();
        render_frame();
        m_camera->update(m_time_passed);
        check_octree_collisions();
        update_time();
    }
}

void ExampleApp::update_time() {
    using namespace std::chrono;
    const auto current_time = high_resolution_clock::now();
    const auto m_time_duration = duration<float, seconds::period>(current_time - m_last_time).count();
    m_last_time = current_time;
}

void ExampleApp::render_frame() {
    if (m_window->check_resize()) {
        recreate_swapchain();
        return;
    }
    m_rendergraph->render();
}

void ExampleApp::check_octree_collisions() {
    // TODO: Implement!
}

void ExampleApp::setup_render_graph() {
    spdlog::trace("Setting up rendergraph");

    m_pipeline_cache = std::make_unique<PipelineCache>(*m_device, "vulkan_pipeline_cache.bin");

    m_rendergraph = std::make_shared<RenderGraph>(*m_device, *m_pipeline_cache);

    const auto swapchain_img_extent = m_swapchain->extent();
    m_back_buffer =
        m_rendergraph->add_texture("Back", TextureUsage::COLOR_ATTACHMENT, m_swapchain->image_format(),
                                   swapchain_img_extent.width, swapchain_img_extent.height, VK_SAMPLE_COUNT_1_BIT);

    m_depth_buffer =
        m_rendergraph->add_texture("Depth", TextureUsage::DEPTH_ATTACHMENT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                   swapchain_img_extent.width, swapchain_img_extent.height, VK_SAMPLE_COUNT_1_BIT);

    m_camera = std::make_shared<Camera>(glm::vec3{0.0f, 0.0f, 0.0f}, 0.0f, 0.0f,
                                        static_cast<float>(swapchain_img_extent.width),
                                        static_cast<float>(swapchain_img_extent.height));

    m_octree_renderer = std::make_unique<ColoredTrianglesOctreeRenderer>(m_rendergraph, m_back_buffer, m_depth_buffer);

    m_octree_renderer->set_camera(m_camera);

    m_imgui_renderer = std::make_unique<ImGuiRenderer>(m_rendergraph, m_octree_renderer->get_pass(), m_swapchain,
                                                       [&]() { update_imgui(); });
}

void ExampleApp::setup_octree_rendering() {
    // Create 3 random worlds
    m_octrees.clear();
    m_octrees.push_back(world::create_random_world(2, {0.0f, 0.0f, 0.0f}, std::optional(42)));
    m_octrees.push_back(world::create_random_world(2, {10.0f, 0.0f, 0.0f}, std::optional(60)));
    m_octrees.push_back(world::create_random_world(2, {20.0f, 0.0f, 0.0f}, std::optional(30)));

    // Register all octrees that were created to the octree renderer
    for (const auto &octree : m_octrees) {
        // m_octree_renderer->add_octree(octree);
    }
}

void ExampleApp::update_imgui() {
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = m_time_passed + 0.00001f;
    auto cursor_pos = m_input_data.get_cursor_pos();
    io.MousePos = ImVec2(static_cast<float>(cursor_pos[0]), static_cast<float>(cursor_pos[1]));
    io.MouseDown[0] = m_input_data.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
    io.MouseDown[1] = m_input_data.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
    io.DisplaySize =
        ImVec2(static_cast<float>(m_swapchain->extent().width), static_cast<float>(m_swapchain->extent().height));

    ImGui::NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(330, 0));
    ImGui::Begin("Inexor vulkan-renderer", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", m_device->gpu_name().c_str());
    ImGui::Text("Engine version %d.%d.%d (Git sha %s)", VK_VERSION_MAJOR(ENGINE_VERSION),
                VK_VERSION_MINOR(ENGINE_VERSION), VK_VERSION_PATCH(ENGINE_VERSION), BUILD_GIT);
    ImGui::Text("Vulkan API %d.%d.%d", VK_API_VERSION_MAJOR(USED_VULKAN_API_VERSION),
                VK_API_VERSION_MINOR(USED_VULKAN_API_VERSION), VK_API_VERSION_PATCH(USED_VULKAN_API_VERSION));
    const auto &cam_pos = m_camera->position();
    ImGui::Text("Camera position (%.2f, %.2f, %.2f)", cam_pos.x, cam_pos.y, cam_pos.z);
    const auto &cam_rot = m_camera->rotation();
    ImGui::Text("Camera rotation: (%.2f, %.2f, %.2f)", cam_rot.x, cam_rot.y, cam_rot.z);
    const auto &cam_front = m_camera->front();
    ImGui::Text("Camera vector front: (%.2f, %.2f, %.2f)", cam_front.x, cam_front.y, cam_front.z);
    const auto &cam_right = m_camera->right();
    ImGui::Text("Camera vector right: (%.2f, %.2f, %.2f)", cam_right.x, cam_right.y, cam_right.z);
    const auto &cam_up = m_camera->up();
    ImGui::Text("Camera vector up (%.2f, %.2f, %.2f)", cam_up.x, cam_up.y, cam_up.z);
    ImGui::Text("Yaw: %.2f pitch: %.2f roll: %.2f", m_camera->yaw(), m_camera->pitch(), m_camera->roll());
    const auto cam_fov = m_camera->fov();
    ImGui::Text("Field of view: %d", static_cast<std::uint32_t>(cam_fov));
    ImGui::PushItemWidth(150.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
}

VkBool32 ExampleApp::validation_layer_debug_messenger_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                               const VkDebugUtilsMessageTypeFlagsEXT type,
                                                               const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                               void *user_data) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        spdlog::trace("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        spdlog::info("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::critical("{}", data->pMessage);
        // TODO: Respect --stop-on-validation-error command line argument!
    }
    return VK_FALSE;
}

void ExampleApp::cursor_position_callback(GLFWwindow *, double, double) {}
void ExampleApp::keyboard_button_callback(GLFWwindow *, int, int, int, int) {}
void ExampleApp::mouse_button_callback(GLFWwindow *, int, int, int) {}
void ExampleApp::mouse_scroll_callback(GLFWwindow *, double, double) {}

} // namespace inexor::vulkan_renderer::example_app

int main(int argc, char *argv[]) {
    try {
        using inexor::vulkan_renderer::example_app::ExampleApp;
        std::unique_ptr<ExampleApp> my_renderer = std::make_unique<ExampleApp>(argc, argv);
        my_renderer->run();
    } catch (std::exception &exception) {
        spdlog::critical(exception.what());
    }
    return 0;
}
