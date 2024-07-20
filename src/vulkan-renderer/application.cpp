#include "inexor/vulkan-renderer/application.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/meta.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/tools/cla_parser.hpp"
#include "inexor/vulkan-renderer/vk_tools/enumerate.hpp"
#include "inexor/vulkan-renderer/world/collision.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <toml.hpp>

#include <random>
#include <thread>

namespace inexor::vulkan_renderer {

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                        const VkDebugUtilsMessageTypeFlagsEXT type,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                        void *user_data) {
    // Validation layers get their own logger
    std::shared_ptr<spdlog::logger> m_validation_log{spdlog::default_logger()->clone("validation")};

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        m_validation_log->trace("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        m_validation_log->info("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        m_validation_log->warn("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        m_validation_log->critical("{}", data->pMessage);
    }
    return false;
}

Application::Application(int argc, char **argv) {
    initialize_spdlog();

    spdlog::trace("Initialising vulkan-renderer");

    tools::CommandLineArgumentParser cla_parser;
    cla_parser.parse_args(argc, argv);

    spdlog::trace("Application version: {}.{}.{}", APP_VERSION[0], APP_VERSION[1], APP_VERSION[2]);
    spdlog::trace("Engine version: {}.{}.{}", ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2]);

    load_toml_configuration_file("configuration/renderer.toml");

    // If the user specified command line argument "--no-validation", the Khronos validation instance layer will be
    // disabled. For debug builds, this is not advisable! Always use validation layers during development!
    const auto disable_validation = cla_parser.arg<bool>("--no-validation");
    if (disable_validation.value_or(false)) {
        spdlog::warn("--no-validation specified, disabling validation layers");
        m_enable_validation_layers = false;
    }

    m_window = std::make_unique<wrapper::Window>(m_wnd_title, m_wnd_width, m_wnd_height, true, true, m_wnd_mode);

    spdlog::trace("Creating Vulkan instance");

    // Management of VkDebugUtilsMessengerCallbackDataEXT is part of the instance wrapper class
    m_instance = std::make_unique<wrapper::Instance>(
        APP_NAME, ENGINE_NAME, VK_MAKE_API_VERSION(0, APP_VERSION[0], APP_VERSION[1], APP_VERSION[2]),
        VK_MAKE_API_VERSION(0, ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2]), m_enable_validation_layers,
        debug_messenger_callback);

    m_input_data = std::make_unique<input::KeyboardMouseInputData>();

    m_surface = std::make_unique<wrapper::WindowSurface>(m_instance->instance(), m_window->get());

    setup_window_and_input_callbacks();

#ifndef NDEBUG
    if (cla_parser.arg<bool>("--stop-on-validation-message").value_or(false)) {
        spdlog::warn("--stop-on-validation-message specified. Application will call a breakpoint after reporting a "
                     "validation layer message");
        m_stop_on_validation_message = true;
    }
#endif

    spdlog::trace("Creating window surface");

    // The user can specify with "--gpu <index>" which graphics card to prefer (index starts from 0)
    auto preferred_graphics_card = cla_parser.arg<std::uint32_t>("--gpu");
    if (preferred_graphics_card) {
        spdlog::trace("Preferential gpu index {} specified", *preferred_graphics_card);
    } else {
        spdlog::trace("No user preferred gpu index specified");
    }

    // If the user specified command line argument "--vsync", the presentation engine waits
    // for the next vertical blanking period to update the current image.
    const auto enable_vertical_synchronisation = cla_parser.arg<bool>("--vsync");
    if (enable_vertical_synchronisation.value_or(false)) {
        spdlog::trace("V-sync enabled!");
        m_vsync_enabled = true;
    } else {
        spdlog::trace("V-sync disabled!");
        m_vsync_enabled = false;
    }

    bool use_distinct_data_transfer_queue = true;

    // Ignore distinct data transfer queue
    const auto forbid_distinct_data_transfer_queue = cla_parser.arg<bool>("--no-separate-data-queue");
    if (forbid_distinct_data_transfer_queue.value_or(false)) {
        spdlog::warn("Command line argument --no-separate-data-queue specified");
        spdlog::warn("This will force the application to avoid using a distinct queue for data transfer to GPU");
        spdlog::warn("Performance loss might be a result of this!");
        use_distinct_data_transfer_queue = false;
    }

    const auto physical_devices = vk_tools::get_physical_devices(m_instance->instance());
    if (preferred_graphics_card && *preferred_graphics_card >= physical_devices.size()) {
        spdlog::critical("GPU index {} out of range!", *preferred_graphics_card);
        throw std::runtime_error("Invalid GPU index");
    }

    const VkPhysicalDeviceFeatures required_features{
        // Add required physical device features here
        .sampleRateShading = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
    };

    const VkPhysicalDeviceFeatures optional_features{
        // Add optional physical device features here
        // TODO: Add callback on_device_feature_not_available and remove optional features
        // Then, return true or false from the callback, indicating if you can run the app
        // without this physical device feature being present.
    };

    // TODO: Also implement a callback for required extensions
    std::vector<const char *> required_extensions{
        // Since we want to draw on a window, we need the swapchain extension
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, // VK_KHR_swapchain
        // We are using dynamic rendering
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, // VK_KHR_dynamic_rendering
        // The following is required by VK_KHR_dynamic_rendering
        VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, // VK_KHR_depth_stencil_resolve
        // The following is required by VK_KHR_depth_stencil_resolve
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, // VK_KHR_create_renderpass2
    };

    const VkPhysicalDevice physical_device =
        preferred_graphics_card ? physical_devices[*preferred_graphics_card]
                                : wrapper::Device::pick_best_physical_device(*m_instance, m_surface->get(),
                                                                             required_features, required_extensions);

    // TODO: Implement on_extension_unavailable and on_feature_unavailable callback
    m_device =
        std::make_unique<wrapper::Device>(*m_instance, m_surface->get(), use_distinct_data_transfer_queue,
                                          physical_device, required_extensions, required_features, optional_features);

    // TODO: Replace ->get() methods with private fields and friend class declaration!
    // TODO: API style like this: m_swapchain = m_device->create_swapchain(m_surface, m_window, m_vsync_enabled);
    m_swapchain = std::make_unique<wrapper::Swapchain>(*m_device, "Default", m_surface->get(), m_window->width(),
                                                       m_window->height(), m_vsync_enabled);

    load_octree_geometry(true);
    generate_octree_indices();

    m_camera = std::make_unique<Camera>(glm::vec3(6.0f, 10.0f, 2.0f), 180.0f, 0.0f,
                                        static_cast<float>(m_window->width()), static_cast<float>(m_window->height()));

    m_camera->set_movement_speed(5.0f);
    m_camera->set_rotation_speed(0.5f);
    m_window->show();
    recreate_swapchain();
}

Application::~Application() {
    spdlog::trace("Shutting down vulkan renderer");
}

void Application::check_octree_collisions() {
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

void Application::cursor_position_callback(GLFWwindow * /*window*/, double x_pos, double y_pos) {
    m_input_data->set_cursor_pos(x_pos, y_pos);
}

void Application::generate_octree_indices() {
    auto old_vertices = std::move(m_octree_vertices);
    m_octree_indices.clear();
    m_octree_vertices.clear();
    std::unordered_map<OctreeGpuVertex, std::uint32_t> vertex_map;
    for (auto &vertex : old_vertices) {
        // TODO: Use std::unordered_map::contains() when we switch to C++ 20.
        if (vertex_map.count(vertex) == 0) {
            assert(vertex_map.size() < std::numeric_limits<std::uint32_t>::max() && "Octree too big!");
            vertex_map.emplace(vertex, static_cast<std::uint32_t>(vertex_map.size()));
            m_octree_vertices.push_back(vertex);
        }
        m_octree_indices.push_back(vertex_map.at(vertex));
    }
    spdlog::trace("Reduced octree by {} vertices (from {} to {})", old_vertices.size() - m_octree_vertices.size(),
                  old_vertices.size(), m_octree_vertices.size());
    spdlog::trace("Total indices {} ", m_octree_indices.size());
}

void Application::initialize_spdlog() {
    spdlog::init_thread_pool(8192, 2);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("vulkan-renderer.log", true);
    auto vulkan_renderer_log =
        std::make_shared<spdlog::async_logger>("vulkan-renderer", spdlog::sinks_init_list{console_sink, file_sink},
                                               spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    vulkan_renderer_log->set_level(spdlog::level::trace);
    vulkan_renderer_log->set_pattern("%Y-%m-%d %T.%f %^%l%$ %5t [%-10n] %v");
    vulkan_renderer_log->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(vulkan_renderer_log);

    spdlog::trace("Inexor vulkan-renderer, BUILD " + std::string(__DATE__) + ", " + __TIME__);
}

void Application::key_callback(GLFWwindow * /*window*/, int key, int, int action, int /*mods*/) {
    if (key < 0 || key > GLFW_KEY_LAST) {
        return;
    }

    switch (action) {
    case GLFW_PRESS:
        m_input_data->press_key(key);
        break;
    case GLFW_RELEASE:
        m_input_data->release_key(key);
        break;
    default:
        break;
    }
}

void Application::load_toml_configuration_file(const std::string &file_name) {
    spdlog::trace("Loading TOML configuration file: {}", file_name);

    std::ifstream toml_file(file_name, std::ios::in);
    if (!toml_file) {
        // If you are using CLion, go to "Edit Configurations" and select "Working Directory".
        throw std::runtime_error("Could not find configuration file: " + file_name +
                                 "! You must set the working directory properly in your IDE");
    }

    toml_file.close();

    // Load the TOML file using toml11.
    auto renderer_configuration = toml::parse(file_name);

    // Search for the title of the configuration file and print it to debug output.
    const auto &configuration_title = toml::find<std::string>(renderer_configuration, "title");
    spdlog::trace("Title: {}", configuration_title);

    using WindowMode = ::inexor::vulkan_renderer::wrapper::Window::Mode;
    const auto &wmodestr = toml::find<std::string>(renderer_configuration, "application", "window", "mode");
    if (wmodestr == "windowed") {
        m_wnd_mode = WindowMode::WINDOWED;
    } else if (wmodestr == "windowed_fullscreen") {
        m_wnd_mode = WindowMode::WINDOWED_FULLSCREEN;
    } else if (wmodestr == "fullscreen") {
        m_wnd_mode = WindowMode::FULLSCREEN;
    } else {
        spdlog::warn("Invalid application window mode: {}", wmodestr);
        m_wnd_mode = WindowMode::WINDOWED;
    }

    m_wnd_width = toml::find<int>(renderer_configuration, "application", "window", "width");
    m_wnd_height = toml::find<int>(renderer_configuration, "application", "window", "height");
    m_wnd_title = toml::find<std::string>(renderer_configuration, "application", "window", "name");
    spdlog::trace("Window: {}, {} x {}", m_wnd_title, m_wnd_width, m_wnd_height);

    m_gltf_model_files = toml::find<std::vector<std::string>>(renderer_configuration, "glTFmodels", "files");

    spdlog::trace("glTF 2.0 models:");

    for (const auto &gltf_model_file : m_gltf_model_files) {
        spdlog::trace("   - {}", gltf_model_file);
    }
}

void Application::load_octree_geometry(bool initialize) {
    spdlog::trace("Creating octree geometry");

    // 4: 23 012 | 5: 184352 | 6: 1474162 | 7: 11792978 cubes, DO NOT USE 7!
    m_worlds.clear();
    m_worlds.push_back(
        world::create_random_world(2, {0.0f, 0.0f, 0.0f}, initialize ? std::optional(42) : std::nullopt));
    m_worlds.push_back(
        world::create_random_world(2, {10.0f, 0.0f, 0.0f}, initialize ? std::optional(60) : std::nullopt));

    m_octree_vertices.clear();
    for (const auto &world : m_worlds) {
        for (const auto &polygons : world->polygons(true)) {
            for (const auto &triangle : *polygons) {
                for (const auto &vertex : triangle) {
                    glm::vec3 color = {
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    };
                    m_octree_vertices.emplace_back(vertex, color);
                }
            }
        }
    }
}

void Application::mouse_button_callback(GLFWwindow * /*window*/, int button, int action, int /*mods*/) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
        return;
    }

    switch (action) {
    case GLFW_PRESS:
        m_input_data->press_mouse_button(button);
        break;
    case GLFW_RELEASE:
        m_input_data->release_mouse_button(button);
        break;
    default:
        break;
    }
}

void Application::mouse_scroll_callback(GLFWwindow * /*window*/, double /*x_offset*/, double y_offset) {
    m_camera->change_zoom(static_cast<float>(y_offset));
}

void Application::process_keyboard_input() {}

void Application::process_mouse_input() {
    const auto cursor_pos_delta = m_input_data->calculate_cursor_position_delta();

    if (m_camera->type() == CameraType::LOOK_AT && m_input_data->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_camera->rotate(static_cast<float>(cursor_pos_delta[0]), -static_cast<float>(cursor_pos_delta[1]));
    }

    m_camera->set_movement_state(CameraMovement::FORWARD, m_input_data->is_key_pressed(GLFW_KEY_W));
    m_camera->set_movement_state(CameraMovement::LEFT, m_input_data->is_key_pressed(GLFW_KEY_A));
    m_camera->set_movement_state(CameraMovement::BACKWARD, m_input_data->is_key_pressed(GLFW_KEY_S));
    m_camera->set_movement_state(CameraMovement::RIGHT, m_input_data->is_key_pressed(GLFW_KEY_D));
}

void Application::recreate_swapchain() {
    m_window->wait_for_focus();
    m_device->wait_idle();

    // Query the framebuffer size here again although the window width is set during framebuffer resize callback
    // The reason for this is that the framebuffer size could already be different again because we missed a poll
    // This seems to be an issue on Linux only though
    int wnd_width = 0;
    int wnd_height = 0;
    glfwGetFramebufferSize(m_window->get(), &wnd_width, &wnd_height);

    m_swapchain->setup(wnd_width, wnd_height, m_vsync_enabled);

    // TODO: Unified API style like this: m_device->create_rendergraph(m_swapchain);
    // TODO: Maybe make RenderGraph constructor (and others) private and only allow device wrapper to call it?
    m_render_graph = std::make_unique<render_graph::RenderGraph>(*m_device);

    setup_render_graph();
}

void Application::render_frame() {
    if (m_wnd_resized) {
        m_wnd_resized = false;
        recreate_swapchain();
        return;
    }

    m_swapchain->acquire_next_image_index();

    m_render_graph->render();

    m_swapchain->present();

    if (auto fps_value = m_fps_counter.update()) {
        m_window->set_title("Inexor Vulkan API renderer demo - " + std::to_string(*fps_value) + " FPS");
        spdlog::trace("FPS: {}, window size: {} x {}", *fps_value, m_window->width(), m_window->height());
    }
}

void Application::run() {
    spdlog::trace("Running Application");

    while (!m_window->should_close()) {
        m_window->poll();
        process_keyboard_input();
        process_mouse_input();
        m_camera->update(m_time_passed);
        m_time_passed = m_stopwatch.time_step();
        check_octree_collisions();
        render_frame();
    }
}

void Application::setup_render_graph() {
    const auto swapchain_extent = m_swapchain->extent();

    m_color_attachment = m_render_graph->add_texture(
        "Color", render_graph::TextureUsage::COLOR_ATTACHMENT, m_swapchain->image_format(), swapchain_extent.width,
        swapchain_extent.height /*, m_device->get_max_usable_sample_count() */);

    m_depth_attachment = m_render_graph->add_texture(
        "Depth", render_graph::TextureUsage::DEPTH_ATTACHMENT, VK_FORMAT_D32_SFLOAT_S8_UINT, swapchain_extent.width,
        swapchain_extent.height /*, m_device->get_max_usable_sample_count()*/);

    m_vertex_buffer = m_render_graph->add_buffer("Octree|Vertex", render_graph::BufferType::VERTEX_BUFFER, [&]() {
        // If the key N was pressed once, generate a new octree
        if (m_input_data->was_key_pressed_once(GLFW_KEY_N)) {
            load_octree_geometry(false);
            generate_octree_indices();
        }
        // Request update of the octree vertex buffer
        m_vertex_buffer.lock()->request_update(m_octree_vertices);
    });

    m_octree_vert = std::make_shared<wrapper::Shader>(*m_device, "Octree|Vert", VK_SHADER_STAGE_VERTEX_BIT,
                                                      "shaders/main.vert.spv");
    m_octree_frag = std::make_shared<wrapper::Shader>(*m_device, "Octree|Frag", VK_SHADER_STAGE_FRAGMENT_BIT,
                                                      "shaders/main.frag.spv");

    // Note that the index buffer is updated together with the vertex buffer to keep data consistent
    // This means for m_index_buffer, on_init and on_update are defaulted to std::nullopt here!
    m_index_buffer = m_render_graph->add_buffer("Octree|Index", render_graph::BufferType::INDEX_BUFFER, [&]() {
        // Request update of the octree index buffer
        m_index_buffer.lock()->request_update(m_octree_indices);
    });

    // TODO: This must be in the init() method of some OctreeRenderer class in the future!
    /// Initialize octree vertices and indices here
    load_octree_geometry(false);
    generate_octree_indices();
    m_vertex_buffer.lock()->request_update(m_octree_vertices);
    m_index_buffer.lock()->request_update(m_octree_indices);

    m_uniform_buffer = m_render_graph->add_buffer("Octree|Uniform", render_graph::BufferType::UNIFORM_BUFFER, [&]() {
        m_mvp_matrices.view = m_camera->view_matrix();
        m_mvp_matrices.proj = m_camera->perspective_matrix();
        m_mvp_matrices.proj[1][1] *= -1;
        m_uniform_buffer.lock()->request_update(m_mvp_matrices);
    });

    m_render_graph->add_resource_descriptor(
        [&](wrapper::descriptors::DescriptorSetLayoutBuilder &builder) {
            m_descriptor_set_layout = builder.add_uniform_buffer(VK_SHADER_STAGE_VERTEX_BIT).build("Octree");
        },
        [&](wrapper::descriptors::DescriptorSetAllocator &allocator) {
            m_descriptor_set = allocator.allocate("Octree", m_descriptor_set_layout);
        },
        [&](wrapper::descriptors::WriteDescriptorSetBuilder &builder) {
            return builder.add_uniform_buffer_update(m_descriptor_set, m_uniform_buffer).build();
        });

    m_render_graph->add_graphics_pipeline([&](wrapper::pipelines::GraphicsPipelineBuilder &builder) {
        m_octree_pipeline = builder
                                .set_vertex_input_bindings({
                                    {
                                        .binding = 0,
                                        .stride = sizeof(OctreeGpuVertex),
                                        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                                    },
                                })
                                // TODO: Fix me!
                                //.set_multisampling(m_device->get_max_usable_sample_count(), 0.25f)
                                .add_default_color_blend_attachment()
                                .add_color_attachment_format(m_swapchain->image_format())
                                // TODO: Implement m_device->get_available_depth_format()
                                .set_depth_attachment_format(VK_FORMAT_D32_SFLOAT_S8_UINT)
                                .set_depth_stencil({.depthTestEnable = VK_TRUE,
                                                    .depthWriteEnable = VK_TRUE,
                                                    .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
                                                    .back{
                                                        .compareOp = VK_COMPARE_OP_ALWAYS,
                                                    }})
                                .set_vertex_input_attributes({
                                    {
                                        .location = 0,
                                        .binding = 0,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(OctreeGpuVertex, position),
                                    },
                                    {
                                        .location = 1,
                                        .binding = 0,
                                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                                        .offset = offsetof(OctreeGpuVertex, color),
                                    },
                                })
                                .set_viewport(m_swapchain->extent())
                                .set_scissor(m_swapchain->extent())
                                .set_descriptor_set_layout(m_descriptor_set_layout)
                                .add_shader(m_octree_vert)
                                .add_shader(m_octree_frag)
                                .build("Octree");
        return m_octree_pipeline;
    });

    m_render_graph->add_graphics_pass([&](render_graph::GraphicsPassBuilder &builder) {
        // NOTE: Octree pass is the first pass, so it does not declare any reads_from()
        m_octree_pass = builder
                            // TODO: Helper function for clear values
                            .writes_to(m_swapchain,
                                       VkClearValue{
                                           .color = {1.0f, 1.0f, 1.0f, 1.0f},
                                       })
                            // TODO: Helper function for clear values
                            .writes_to(m_depth_attachment,
                                       VkClearValue{
                                           .depthStencil =
                                               VkClearDepthStencilValue{
                                                   .depth = 1.0f,
                                               },
                                       })
                            .set_on_record([&](const wrapper::commands::CommandBuffer &cmd_buf) {
                                cmd_buf.bind_pipeline(m_octree_pipeline)
                                    .bind_descriptor_set(m_descriptor_set, m_octree_pipeline)
                                    .bind_vertex_buffer(m_vertex_buffer)
                                    .bind_index_buffer(m_index_buffer)
                                    .draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
                            })
                            .build("Octree", render_graph::DebugLabelColor::RED);
        return m_octree_pass;
    });

    // TODO: We don't need to recreate the imgui overlay when swapchain is recreated, use a .recreate() method instead?
    // TODO: Decouple ImGuiRenderer form ImGuiLoader
    m_imgui_overlay = std::make_unique<renderers::ImGuiRenderer>(*m_device, m_render_graph, m_octree_pass, m_swapchain,
                                                                 [&]() { update_imgui_overlay(); });

    m_render_graph->compile();
}

void Application::setup_window_and_input_callbacks() {
    // The following code requires some explanation
    // Because glfw is a C-style API, we can't use a pointer to non-static class methods as window or input callbacks.
    // For example, we can't use Application::key_callback in glfwSetKeyCallback as key callback directly.
    // A good explanation can be found on Stack Overflow:
    // https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
    // In order to fix this, we can pass a lambda to glfwSetKeyCallback, which calls Application::key_callback
    // internally. But there is another problem: Inside of the template, we need to call Application::Key_callback. In
    // order to do so, we need to have access to the this-pointer. Unfortunately, the this-pointer can't be captured
    // in the lambda capture like [this](){}, because the glfw would not accept the lambda then. To work around this
    // problem, we store the this pointer using glfwSetWindowUserPointer. Inside of these lambdas, we then cast the
    // pointer to Application* again, allowing us to finally use the callbacks.

    m_window->set_user_ptr(this);

    spdlog::trace("Setting up window callback:");

    auto lambda_frame_buffer_resize_callback = [](GLFWwindow *window, int width, int height) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        spdlog::trace("Frame buffer resize callback called. window width: {}, height: {}", width, height);
        app->m_wnd_resized = true;
    };

    m_window->set_resize_callback(lambda_frame_buffer_resize_callback);

    spdlog::trace("   - keyboard button callback");

    auto lambda_key_callback = [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->key_callback(window, key, scancode, action, mods);
    };

    m_window->set_keyboard_button_callback(lambda_key_callback);

    spdlog::trace("   - cursor position callback");

    auto lambda_cursor_position_callback = [](GLFWwindow *window, double xpos, double ypos) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->cursor_position_callback(window, xpos, ypos);
    };

    m_window->set_cursor_position_callback(lambda_cursor_position_callback);

    spdlog::trace("   - mouse button callback");

    auto lambda_mouse_button_callback = [](GLFWwindow *window, int button, int action, int mods) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->mouse_button_callback(window, button, action, mods);
    };

    m_window->set_mouse_button_callback(lambda_mouse_button_callback);

    spdlog::trace("   - mouse wheel scroll callback");

    auto lambda_mouse_scroll_callback = [](GLFWwindow *window, double xoffset, double yoffset) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->mouse_scroll_callback(window, xoffset, yoffset);
    };

    m_window->set_mouse_scroll_callback(lambda_mouse_scroll_callback);
}

void Application::update_imgui_overlay() {
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = m_time_passed + 0.00001f;
    auto cursor_pos = m_input_data->get_cursor_pos();
    io.MousePos = ImVec2(static_cast<float>(cursor_pos[0]), static_cast<float>(cursor_pos[1]));
    io.MouseDown[0] = m_input_data->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
    io.MouseDown[1] = m_input_data->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
    io.DisplaySize =
        ImVec2(static_cast<float>(m_swapchain->extent().width), static_cast<float>(m_swapchain->extent().height));

    ImGui::NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(330, 0));
    ImGui::Begin("Inexor Vulkan-renderer", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", m_device->gpu_name().c_str());
    ImGui::Text("Engine version %d.%d.%d (Git sha %s)", ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2],
                BUILD_GIT);
    ImGui::Text("Vulkan API %d.%d.%d", VK_API_VERSION_MAJOR(wrapper::Instance::REQUIRED_VK_API_VERSION),
                VK_API_VERSION_MINOR(wrapper::Instance::REQUIRED_VK_API_VERSION),
                VK_API_VERSION_PATCH(wrapper::Instance::REQUIRED_VK_API_VERSION));
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

} // namespace inexor::vulkan_renderer
