#include "application.hpp"

#include "inexor/vulkan-renderer/input/gamepad_data.hpp"
#include "inexor/vulkan-renderer/input/input.hpp"
#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"
#include "inexor/vulkan-renderer/meta/meta.hpp"
#include "inexor/vulkan-renderer/octree/collision.hpp"
#include "inexor/vulkan-renderer/octree/collision_query.hpp"
#include "inexor/vulkan-renderer/octree/cube.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/tools/device_info.hpp"
#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/random.hpp"
#include "inexor/vulkan-renderer/wrapper/core/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/windows/surface.hpp"
#include "inexor/vulkan-renderer/wrapper/windows/window.hpp"

#include <CLI/CLI.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <vk_mem_alloc.h>

#include <mutex>
#include <stdexcept>
#include <string_view>
#include <toml++/toml.hpp>
#include <unordered_map>

namespace inexor::example_app {

// Using declarations
using namespace inexor::vulkan_renderer;

namespace {

void log_vma_statistics(const wrapper::core::Device &device, std::string_view context) {
    char *vma_stats_string = nullptr;
    vmaBuildStatsString(device.allocator(), &vma_stats_string, VK_TRUE);
    if (vma_stats_string == nullptr) {
        spdlog::warn("[{}] VMA statistics are unavailable", context);
        return;
    }

    spdlog::info("[{}] VMA memory statistics:\n{}", context, vma_stats_string);
    vmaFreeStatsString(device.allocator(), vma_stats_string);
}

} // namespace

void ExampleApp::load_toml_configuration_file(const std::string &file_name) {
    spdlog::trace("Loading TOML configuration file: {}", file_name);

    // Load the TOML file using tomlplusplus.
    auto config_file = toml::parse_file(file_name);

    const std::string_view project_title = config_file["title"].value_or("");
    spdlog::trace("Title: {}", project_title);

    const std::string_view wnd_mode = config_file["application"]["window"]["mode"].value_or("windowed");

    if (wnd_mode == "windowed") {
        m_window_mode = WindowMode::WINDOWED;
    } else if (wnd_mode == "windowed_fullscreen") {
        m_window_mode = WindowMode::WINDOWED_FULLSCREEN;
    } else if (wnd_mode == "fullscreen") {
        m_window_mode = WindowMode::FULLSCREEN;
    } else {
        spdlog::warn("Invalid application window mode: {}", wnd_mode);
        m_window_mode = WindowMode::WINDOWED;
    }

    m_window_width = config_file["application"]["window"]["width"].value_or(1280);
    m_window_height = config_file["application"]["window"]["height"].value_or(720);
    m_window_title = config_file["application"]["window"]["name"].value_or("Undefined Window Title!");
    spdlog::trace("Window: {}, {} x {}", m_window_title, m_window_width, m_window_height);
}

VkBool32 ExampleApp::validation_layer_debug_messenger_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                               const VkDebugUtilsMessageTypeFlagsEXT type,
                                                               const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                               void *user_data) {
    // Use different spdlog methods based on the severity of the message
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        spdlog::trace("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        spdlog::info("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::critical("{}", data->pMessage);
    }
    return VK_FALSE;
}

void ExampleApp::load_octree_geometry(bool initialize) {
    const auto old_vertex_count = m_octree_vertices.size();

    // 4: 23 012 | 5: 184352 | 6: 1474162 | 7: 11792978 cubes, DO NOT USE 7!
    m_worlds.clear();
    using octree::create_random_world;
    m_worlds.push_back(create_random_world(2, {0.0f, 0.0f, 0.0f}, initialize ? std::optional(42) : std::nullopt));
    m_worlds.push_back(create_random_world(2, {10.0f, 0.0f, 0.0f}, initialize ? std::optional(60) : std::nullopt));

    using tools::generate_random_number;
    m_octree_vertices.clear();
    for (const auto &world : m_worlds) {
        for (const auto &polygons : world->polygons(true)) {
            for (const auto &triangle : *polygons) {
                for (const auto &vertex : triangle) {
                    glm::vec3 color = {
                        generate_random_number(0.0f, 1.0f),
                        generate_random_number(0.0f, 1.0f),
                        generate_random_number(0.0f, 1.0f),
                    };
                    m_octree_vertices.emplace_back(vertex, color);
                }
            }
        }
    }
    spdlog::trace("Octree vertices generated [new: {}, old: {}]", m_octree_vertices.size(), old_vertex_count);
}

void ExampleApp::generate_octree_indices() {
    auto old_vertices = std::move(m_octree_vertices);
    m_octree_indices.clear();
    m_octree_vertices.clear();
    std::unordered_map<OctreeVertex, std::uint32_t> vertex_map;
    for (auto &vertex : old_vertices) {
        // TODO: Use std::unordered_map::contains() when we switch to C++ 20.
        if (vertex_map.count(vertex) == 0) {
            if (vertex_map.size() >= std::numeric_limits<std::uint32_t>::max()) {
                throw std::overflow_error("Octree too big!");
            }
            vertex_map.emplace(vertex, static_cast<std::uint32_t>(vertex_map.size()));
            m_octree_vertices.push_back(vertex);
        }
        m_octree_indices.push_back(vertex_map.at(vertex));
    }
    // @TODO Fix index generation and bring back debug output
}

void ExampleApp::setup_window_and_input_callbacks() {
    m_window->set_user_ptr(this);

    spdlog::trace("Setting up window callback:");
    auto lambda_frame_buffer_resize_callback = [](GLFWwindow *window, int width, int height) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->m_window_resized = true;
    };
    m_window->set_resize_callback(lambda_frame_buffer_resize_callback);

    spdlog::trace("   - keyboard button callback");
    auto lambda_key_callback = [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->m_input->key_callback(window, key, scancode, action, mods);
    };
    m_window->set_keyboard_button_callback(lambda_key_callback);

    spdlog::trace("   - cursor position callback");
    auto lambda_cursor_position_callback = [](GLFWwindow *window, double xpos, double ypos) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->m_input->cursor_position_callback(window, xpos, ypos);
    };
    m_window->set_cursor_position_callback(lambda_cursor_position_callback);

    spdlog::trace("   - mouse button callback");
    auto lambda_mouse_button_callback = [](GLFWwindow *window, int button, int action, int mods) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->m_input->mouse_button_callback(window, button, action, mods);
    };
    m_window->set_mouse_button_callback(lambda_mouse_button_callback);

    spdlog::trace("   - mouse wheel scroll callback");
    auto lambda_mouse_scroll_callback = [](GLFWwindow *window, double xoffset, double yoffset) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->m_input->mouse_scroll_callback(window, xoffset, yoffset);
    };
    m_window->set_mouse_scroll_callback(lambda_mouse_scroll_callback);
}

void ExampleApp::initialize_spdlog() {
    // Initialization of spdlog with only one thread should be fine because at no point do we expect many spdlog
    // messages to be written to the console and the logfile.
    spdlog::init_thread_pool(8192, 1);

    // A copy of the console sink will automatically be saved to a logfile
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string(meta::APP_NAME) + ".log", true);

    // We only use one global logger by default instead of one logger for each code component to keep it simple
    auto logger = std::make_shared<spdlog::async_logger>("main", spdlog::sinks_init_list{console_sink, file_sink},
                                                         spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::set_default_logger(logger);

    logger->flush_on(spdlog::level::trace);
    logger->set_level(spdlog::level::trace);
    logger->set_pattern("%Y-%m-%d %T.%f %^%l%$ %5t [%n] %v");
}

ExampleApp::ExampleApp(int argc, char **argv) {
    initialize_spdlog();

    // Print some metadata about the project and build to console
    using namespace vulkan_renderer::meta;
    spdlog::trace("{}", APP_NAME);
    spdlog::trace("Application version: {}", APP_VERSION_STR);
    spdlog::trace("Engine version: {}", ENGINE_VERSION_STR);
    spdlog::trace("Build configuration: {}", BUILD_TYPE);
    spdlog::trace("Build date: {}, Time: {}", std::string(__DATE__), std::string(__TIME__));
    spdlog::trace("Git SHA: {}", BUILD_GIT);

    // Parse the command line arguments
    CLI::App app{"vulkan-renderer"};
    argv = app.ensure_utf8(argv);
    app.add_flag("--vsync", m_vsync_enabled);
    app.add_flag("--no-cmd-buf-cache", m_no_cmd_buf_cache);
    std::optional<std::uint32_t> preferred_gpu;
    app.add_option("--gpu", preferred_gpu);
    std::uint32_t max_fps = FPSLimiter::DEFAULT_FPS;
    app.add_option("--maxfps", max_fps);
    std::uint32_t msaa_samples = 1;
    app.add_option("--msaa", msaa_samples);
    app.parse(argc, argv);

    m_fps_limiter.set_max_fps(max_fps);

    spdlog::info("MSAA samples requested: {}", msaa_samples);

    // Convert MSAA sample count to VkSampleCountFlagBits
    switch (msaa_samples) {
    case 1:
        m_msaa_sample_count = VK_SAMPLE_COUNT_1_BIT;
        break;
    case 2:
        m_msaa_sample_count = VK_SAMPLE_COUNT_2_BIT;
        break;
    case 4:
        m_msaa_sample_count = VK_SAMPLE_COUNT_4_BIT;
        break;
    case 8:
        m_msaa_sample_count = VK_SAMPLE_COUNT_8_BIT;
        break;
    case 16:
        m_msaa_sample_count = VK_SAMPLE_COUNT_16_BIT;
        break;
    default:
        spdlog::warn("Invalid MSAA sample count: {}, defaulting to 1", msaa_samples);
        m_msaa_sample_count = VK_SAMPLE_COUNT_1_BIT;
        break;
    }

    spdlog::info("MSAA sample count set to: {}", static_cast<int>(m_msaa_sample_count));

    if (m_msaa_sample_count != VK_SAMPLE_COUNT_1_BIT) {
        spdlog::trace("MSAA requested: {}x", msaa_samples);
    }

    load_toml_configuration_file("assets/configuration/renderer.toml");

    m_window = std::make_unique<Window>(m_window_title, m_window_width, m_window_height, true, true, m_window_mode);

    std::vector<const char *> instance_layers;
    std::vector<const char *> instance_extensions;

    // It is very important to start using Vulkan API by initializing volk with the following function call,
    // otherwise even the most basic Vulkan functions which do not depend on a VkInstance or a VkDevice will not be
    // available!
    spdlog::trace("Initializing volk metaloader");
    if (const auto result = volkInitialize(); result != VK_SUCCESS) {
        throw InexorException("Error: Vulkan initialization with volk metaloader library failed!");
    }

    // If the instance extension "VK_EXT_debug_utils" is available on the system, enable it.
    if (wrapper::core::is_instance_extension_supported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Get the instance extensions which are required by glfw library.
    std::uint32_t glfw_extension_count = 0;
    auto *glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    if (glfw_extension_count == 0) {
        throw InexorException("Error: glfwGetRequiredInstanceExtensions returned 0 required instance extensions!");
    }

    spdlog::trace("Required GLFW instance extensions:");
    for (std::size_t index = 0; index < glfw_extension_count; index++) {
        // We must make sure that each instance extension that is required by glfw is available on the system.
        if (!wrapper::core::is_instance_extension_supported(glfw_extensions[index])) {
            // If any of the instance extensions that is required by glfw is not available, we will fail.
            throw InexorException("Error: glfw instance extension '" + std::string(glfw_extensions[index]) +
                                  "' is not available on the system!");
        } else {
            spdlog::trace("   - {}", glfw_extensions[index]);
            instance_extensions.push_back(glfw_extensions[index]);
        }
    }

    if (wrapper::core::is_instance_layer_supported("VK_LAYER_KHRONOS_validation")) {
        instance_layers.push_back("VK_LAYER_KHRONOS_validation");
    } else {
        spdlog::error("Instance layer 'VK_LAYER_KHRONOS_validation' is not available on this system!");
    }

    spdlog::trace("Creating Vulkan instance");

    m_instance = std::make_unique<Instance>(instance_layers, instance_extensions);
    m_dbg_callback = std::make_unique<VulkanDebugUtilsCallback>(*m_instance, validation_layer_debug_messenger_callback);

    m_input = std::make_unique<Input>();
    setup_window_and_input_callbacks();

    m_surface = std::make_unique<WindowSurface>(m_instance->instance(), m_window->window());

    if (preferred_gpu) {
        spdlog::trace("Preferential graphics card index {} specified", *preferred_gpu);
    }

    spdlog::trace("V-sync {}", m_vsync_enabled ? "enabled" : "disabled");

    const auto physical_devices = tools::get_physical_devices(m_instance->instance());
    if (preferred_gpu && *preferred_gpu >= physical_devices.size()) {
        spdlog::critical("GPU index {} is out of range!", *preferred_gpu);
        // NOTE: This is not a problem, the most suitable gpu will be chosen automatically later!
        preferred_gpu = std::nullopt;
    }

    const VkPhysicalDeviceFeatures required_features{
        // Add required physical device features here if desired...
    };

    std::vector<const char *> required_extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    const VkPhysicalDevice physical_device =
        preferred_gpu ? physical_devices[*preferred_gpu]
                      : tools::pick_best_physical_device(*m_instance, m_surface->surface(), required_features,
                                                         required_extensions);

    m_device = std::make_unique<Device>(*m_instance, m_surface->surface(), physical_device, required_features,
                                        required_extensions);

    // Validate MSAA sample count against depth format capabilities
    if (m_msaa_sample_count != VK_SAMPLE_COUNT_1_BIT) {
        // Query supported sample counts for the depth format
        VkImageFormatProperties image_format_props;
        const VkResult result = vkGetPhysicalDeviceImageFormatProperties(
            physical_device,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            0,
            &image_format_props);

        if (result != VK_SUCCESS) {
            spdlog::error("Failed to query image format properties for depth format");
            m_msaa_sample_count = VK_SAMPLE_COUNT_1_BIT;
        } else {
            const VkSampleCountFlags supported_samples = image_format_props.sampleCounts;

            // Check if the requested sample count is supported
            if (!(supported_samples & m_msaa_sample_count)) {
                // Clamp to the highest supported sample count
                VkSampleCountFlagBits clamped = VK_SAMPLE_COUNT_1_BIT;
                if (supported_samples & VK_SAMPLE_COUNT_64_BIT) {
                    clamped = VK_SAMPLE_COUNT_64_BIT;
                } else if (supported_samples & VK_SAMPLE_COUNT_32_BIT) {
                    clamped = VK_SAMPLE_COUNT_32_BIT;
                } else if (supported_samples & VK_SAMPLE_COUNT_16_BIT) {
                    clamped = VK_SAMPLE_COUNT_16_BIT;
                } else if (supported_samples & VK_SAMPLE_COUNT_8_BIT) {
                    clamped = VK_SAMPLE_COUNT_8_BIT;
                } else if (supported_samples & VK_SAMPLE_COUNT_4_BIT) {
                    clamped = VK_SAMPLE_COUNT_4_BIT;
                } else if (supported_samples & VK_SAMPLE_COUNT_2_BIT) {
                    clamped = VK_SAMPLE_COUNT_2_BIT;
                }
                spdlog::warn("Requested MSAA sample count not supported by depth format, clamping from {} to {}",
                             static_cast<int>(m_msaa_sample_count), static_cast<int>(clamped));
                m_msaa_sample_count = clamped;
            } else {
                spdlog::info("MSAA sample count {} is supported by depth format", static_cast<int>(m_msaa_sample_count));
            }
        }
    }

    m_swapchain = std::make_shared<Swapchain>(*m_device, "m_swapchain", m_surface->surface());

    m_camera = std::make_unique<Camera>(glm::vec3(6.0f, 10.0f, 2.0f), 180.0f, 0.0f,
                                        static_cast<float>(m_window->width()), static_cast<float>(m_window->height()));
    // @TODO Find a balance between exposing too few and too many parameters in Camera class constructor
    m_camera->set_near_plane(0.1f);
    m_camera->set_movement_speed(5.0f);
    m_camera->set_rotation_speed(0.5f);

    m_render_graph = std::make_unique<RenderGraph>(*m_device, !m_no_cmd_buf_cache);

    load_octree_geometry(true);
    generate_octree_indices();

    m_window->show();
    recreate_swapchain();
    setup_render_graph();

    m_octree_renderer->set_vertices_and_indices(m_octree_vertices, m_octree_indices);
}

ExampleApp::~ExampleApp() {}

void ExampleApp::render_frame() {
    if (m_window_resized) {
        m_window_resized = false;
        recreate_swapchain();
        return;
    }

    m_render_graph->render();

    if (auto fps_value = m_fps_limiter.get_fps()) {
        m_window->set_title("Inexor Vulkan API renderer demo - " + std::to_string(*fps_value) + " FPS");
    }
}

void ExampleApp::recreate_swapchain() {
    m_window->wait_for_focus();
    m_device->wait_idle();

    // Query the framebuffer size here again although the window width is set during framebuffer resize callback
    // The reason for this is that the framebuffer size could already be different again because we missed a poll
    // This seems to be an issue on Linux only though
    auto [window_width, window_height] = m_window->get_framebuffer_size();
    m_camera->set_aspect_ratio(window_width, window_height);

    m_swapchain->setup_swapchain(
        VkExtent2D{static_cast<std::uint32_t>(window_width), static_cast<std::uint32_t>(window_height)},
        m_vsync_enabled);

    // @TODO Update or recreate all swapchain or image attachments!
}

void ExampleApp::setup_render_graph() {
    // Create a depth buffer for octree rendering (ImGui pass does not require it)
    m_depth_buffer = m_render_graph->add_texture("m_depth_buffer", TextureUsage::DEPTH_ATTACHMENT,
                                                 VK_FORMAT_D32_SFLOAT_S8_UINT, m_swapchain->extent().width,
                                                 m_swapchain->extent().height, 1, m_msaa_sample_count, [&]() {
                                                     if (const auto depth_buffer = m_depth_buffer.lock()) {
                                                         const auto extent = m_swapchain->extent();
                                                         depth_buffer->request_resize(extent.width, extent.height);
                                                     }
                                                 });

    // Create MSAA color buffer if MSAA is enabled
    if (m_msaa_sample_count != VK_SAMPLE_COUNT_1_BIT) {
        spdlog::info("Creating MSAA color buffer with {} samples", static_cast<int>(m_msaa_sample_count));
        m_color_buffer = m_render_graph->add_texture("m_color_buffer", TextureUsage::COLOR_ATTACHMENT,
                                                     m_swapchain->image_format(), m_swapchain->extent().width,
                                                     m_swapchain->extent().height, 4, m_msaa_sample_count, [&]() {
                                                         if (const auto color_buffer = m_color_buffer.lock()) {
                                                             const auto extent = m_swapchain->extent();
                                                             color_buffer->request_resize(extent.width, extent.height);
                                                         }
                                                     });
    }

    // Initialize the octree renderer
    m_octree_renderer = std::make_unique<OctreeRenderer>(m_render_graph, m_swapchain, m_depth_buffer, m_camera, m_color_buffer);

    // Initialize the ImGui renderer
    m_imgui_renderer = std::make_unique<ImGuiRenderer>(m_render_graph, m_swapchain, [&]() {
        // This is the external user-defined ImGui update function
        update_imgui_overlay();
    });

    m_render_graph->compile();
}

void ExampleApp::update_imgui_overlay() {
    auto cursor_pos = m_input->kbm_data().get_cursor_pos();

    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = m_fps_limiter.elapsed_seconds();
    io.MousePos = ImVec2(static_cast<float>(cursor_pos[0]), static_cast<float>(cursor_pos[1]));
    io.MouseDown[0] = m_input->kbm_data().is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
    io.MouseDown[1] = m_input->kbm_data().is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
    io.DisplaySize =
        ImVec2(static_cast<float>(m_swapchain->extent().width), static_cast<float>(m_swapchain->extent().height));

    ImGui::NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(330, 0));
    using namespace vulkan_renderer::meta;
    ImGui::Begin(APP_NAME, nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", m_device->gpu_name().c_str());
    ImGui::Text("Engine version %s (git SHA %s)", ENGINE_VERSION_STR, BUILD_GIT);
    const char *msaa_text = "No MSAA";
    switch (m_msaa_sample_count) {
    case VK_SAMPLE_COUNT_2_BIT:
        msaa_text = "2xMSAA";
        break;
    case VK_SAMPLE_COUNT_4_BIT:
        msaa_text = "4xMSAA";
        break;
    case VK_SAMPLE_COUNT_8_BIT:
        msaa_text = "8xMSAA";
        break;
    case VK_SAMPLE_COUNT_16_BIT:
        msaa_text = "16xMSAA";
        break;
    case VK_SAMPLE_COUNT_1_BIT:
    default:
        msaa_text = "No MSAA";
        break;
    }
    ImGui::Text("Vulkan API %d.%d.%d, %s", VK_API_VERSION_MAJOR(Instance::REQUIRED_VK_API_VERSION),
                VK_API_VERSION_MINOR(Instance::REQUIRED_VK_API_VERSION),
                VK_API_VERSION_PATCH(Instance::REQUIRED_VK_API_VERSION), msaa_text);
    ImGui::Text("Press N to regenerate octree");
    ImGui::Text("Press V for VMA memory statistics");
    const auto cam_pos = m_camera->position();
    ImGui::Text("Camera position (%.2f, %.2f, %.2f)", cam_pos.x, cam_pos.y, cam_pos.z);
    const auto cam_rot = m_camera->rotation();
    ImGui::Text("Camera rotation: (%.2f, %.2f, %.2f)", cam_rot.x, cam_rot.y, cam_rot.z);
    const auto cam_front = m_camera->front();
    ImGui::Text("Camera vector front: (%.2f, %.2f, %.2f)", cam_front.x, cam_front.y, cam_front.z);
    const auto cam_right = m_camera->right();
    ImGui::Text("Camera vector right: (%.2f, %.2f, %.2f)", cam_right.x, cam_right.y, cam_right.z);
    const auto cam_up = m_camera->up();
    ImGui::Text("Camera vector up (%.2f, %.2f, %.2f)", cam_up.x, cam_up.y, cam_up.z);
    ImGui::Text("Yaw: %.2f pitch: %.2f roll: %.2f", m_camera->yaw(), m_camera->pitch(), m_camera->roll());
    const auto cam_fov = m_camera->fov();
    ImGui::Text("Field of view: %d", static_cast<std::uint32_t>(cam_fov));
    ImGui::PushItemWidth(150.0f);
    ImGui::PopItemWidth();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::Render();
}

void ExampleApp::process_input() {
    const auto cursor_pos_delta = m_input->kbm_data().calculate_cursor_position_delta();

    auto deadzone_lambda = [](const float state) { return (glm::abs(state) < 0.2f) ? 0.0f : state; };

    if (m_camera->type() == CameraType::LOOK_AT &&
        m_input->kbm_data().is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_camera->rotate(static_cast<float>(cursor_pos_delta[0]), -static_cast<float>(cursor_pos_delta[1]));
    }
    if (m_camera->type() == CameraType::LOOK_AT) {
        m_camera->rotate(deadzone_lambda(m_input->gamepad_data().current_joystick_axes(1).x) * 5.f,
                         deadzone_lambda(m_input->gamepad_data().current_joystick_axes(1).y) * -5.f);
    }

    m_camera->set_movement_state(CameraMovement::FORWARD,
                                 m_input->gamepad_data().current_joystick_axes(0)[GLFW_GAMEPAD_AXIS_LEFT_Y] <= -0.15);
    m_camera->set_movement_state(CameraMovement::LEFT,
                                 m_input->gamepad_data().current_joystick_axes(0)[GLFW_GAMEPAD_AXIS_LEFT_X] <= -0.15);
    m_camera->set_movement_state(CameraMovement::BACKWARD,
                                 m_input->gamepad_data().current_joystick_axes(0)[GLFW_GAMEPAD_AXIS_LEFT_Y] >= 0.15);
    m_camera->set_movement_state(CameraMovement::RIGHT,
                                 m_input->gamepad_data().current_joystick_axes(0)[GLFW_GAMEPAD_AXIS_LEFT_X] >= 0.15);
    m_camera->set_movement_state(CameraMovement::FORWARD, m_input->kbm_data().is_key_pressed(GLFW_KEY_W));
    m_camera->set_movement_state(CameraMovement::LEFT, m_input->kbm_data().is_key_pressed(GLFW_KEY_A));
    m_camera->set_movement_state(CameraMovement::BACKWARD, m_input->kbm_data().is_key_pressed(GLFW_KEY_S));
    m_camera->set_movement_state(CameraMovement::RIGHT, m_input->kbm_data().is_key_pressed(GLFW_KEY_D));
    m_camera->update(m_fps_limiter.elapsed_seconds());
}

void ExampleApp::check_octree_collisions() {
    // Check for collision between camera ray and every octree
    for (const auto &world : m_worlds) {
        const auto collision = ray_cube_collision_check(*world, m_camera->position(), m_camera->front());

        if (collision) {
            const auto intersection = collision.value().intersection();
            const auto face_normal = collision.value().face();
            const auto corner = collision.value().corner();
            const auto edge = collision.value().edge();

            spdlog::trace("pos {} {} {} | face {} {} {} | corner {} {} {} | edge {} {} {}", intersection.x,
                          intersection.y, intersection.z, face_normal.x, face_normal.y, face_normal.z, corner.x,
                          corner.y, corner.z, edge.x, edge.y, edge.z);

            // Break after one collision.
            break;
        }
    }
}

void ExampleApp::run() {
    spdlog::trace("Running Application");

    while (!m_window->should_close()) {
        m_window->poll();
        if (m_fps_limiter.is_next_frame_allowed()) {
            m_input->update_gamepad_data();
            process_input();
            update_imgui_overlay();
            render_frame();
            if (m_input->kbm_data().was_key_pressed_once(GLFW_KEY_N)) {
                load_octree_geometry(false);
                generate_octree_indices();
                m_octree_renderer->set_vertices_and_indices(m_octree_vertices, m_octree_indices);
            }
            if (m_input->kbm_data().was_key_pressed_once(GLFW_KEY_V)) {
                log_vma_statistics(*m_device, "Manual VMA statistics");
            }
            check_octree_collisions();
        }
    }
}

} // namespace inexor::example_app
