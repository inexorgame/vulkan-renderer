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
#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"

#include <CLI/CLI.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <string_view>
#include <toml++/toml.hpp>

namespace inexor::example_app {

using namespace inexor::vulkan_renderer;

void ExampleApp::load_toml_configuration_file(const std::string &file_name) {
    spdlog::trace("Loading TOML configuration file: {}", file_name);

    // @TODO Switch to std::filesystem::exists
    std::ifstream toml_file(file_name, std::ios::in);
    if (!toml_file) {
        // If you are using CLion, go to "Edit Configurations" and select "Working Directory".
        throw InexorException("Could not find configuration file: " + file_name +
                              "! You must set the working directory properly in your IDE");
    }

    toml_file.close();

    // Load the TOML file using tomlplusplus.
    auto config_file = toml::parse_file(file_name);

    const std::string_view project_title = config_file["title"].value_or("");
    spdlog::trace("Title: {}", project_title);

    const std::string_view wnd_mode = config_file["application"]["window"]["mode"].value_or("windowed");

    if (wnd_mode == "windowed") {
        m_window_mode = Mode::WINDOWED;
    } else if (wnd_mode == "windowed_fullscreen") {
        m_window_mode = Mode::WINDOWED_FULLSCREEN;
    } else if (wnd_mode == "fullscreen") {
        m_window_mode = Mode::FULLSCREEN;
    } else {
        spdlog::warn("Invalid application window mode: {}", wnd_mode);
        m_window_mode = Mode::WINDOWED;
    }

    m_window_width = config_file["application"]["window"]["width"].value_or(1280);
    m_window_height = config_file["application"]["window"]["height"].value_or(720);
    m_window_title = config_file["application"]["window"]["name"].value_or("Undefined Window Title!");
    spdlog::trace("Window: {}, {} x {}", m_window_title, m_window_width, m_window_height);

    spdlog::trace("Textures:");
    const auto texture_files = config_file["textures"]["files"].as_array();
    for (const auto &value : *texture_files) {
        const auto texture_file = value.value_or("");
        spdlog::trace("   - {}", texture_file);
        m_texture_files.push_back(texture_file);
    }

    spdlog::trace("glTF 2.0 models:");
    const auto gltf_models = config_file["glTFmodels"]["files"].as_array();
    for (const auto &value : *gltf_models) {
        const std::string gltf_model_file = value.value_or("");
        spdlog::trace("   - {}", gltf_model_file);
        m_gltf_model_files.push_back(gltf_model_file);
    }

    spdlog::trace("Vertex shaders:");
    const auto vertex_shader_files = config_file["shaders"]["vertex"]["files"].as_array();
    for (const auto &value : *vertex_shader_files) {
        const std::string vertex_shader_file = value.value_or("");
        spdlog::trace("   - {}", vertex_shader_file);
        m_vertex_shader_files.push_back(vertex_shader_file);
    }

    spdlog::trace("Fragment shaders:");
    const auto fragment_shader_files = config_file["shaders"]["fragment"]["files"].as_array();
    for (const auto &value : *fragment_shader_files) {
        const std::string fragment_shader_file = value.value_or("");
        spdlog::trace("   - {}", fragment_shader_file);
        m_fragment_shader_files.push_back(fragment_shader_file);
    }
}

void ExampleApp::load_shaders() {
    spdlog::trace("Loading vertex shaders:");

    if (m_vertex_shader_files.empty()) {
        spdlog::error("No vertex shaders to load!");
    }

    // Loop through the list of vertex shaders and initialise all of them.
    for (const auto &vertex_shader_file : m_vertex_shader_files) {
        spdlog::trace("   - {}", vertex_shader_file);

        // Insert the new shader into the list of shaders.
        m_shaders.emplace_back(*m_device, VK_SHADER_STAGE_VERTEX_BIT, "unnamed vertex shader", vertex_shader_file);
    }

    spdlog::trace("Loading fragment shaders:");

    if (m_fragment_shader_files.empty()) {
        spdlog::error("No fragment shaders to load!");
    }

    // Loop through the list of fragment shaders and initialise all of them.
    for (const auto &fragment_shader_file : m_fragment_shader_files) {
        spdlog::trace("   - {}", fragment_shader_file);

        // Insert the new shader into the list of shaders.
        m_shaders.emplace_back(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "unnamed fragment shader",
                               fragment_shader_file);
    }

    spdlog::trace("Loading shaders finished");
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
    spdlog::trace("Creating octree geometry");

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
}

void ExampleApp::generate_octree_indices() {
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

void ExampleApp::setup_window_and_input_callbacks() {
    m_window->set_user_ptr(this);

    spdlog::trace("Setting up window callback:");

    auto lambda_frame_buffer_resize_callback = [](GLFWwindow *window, int width, int height) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        spdlog::trace("Frame buffer resize callback called. window width: {}, height: {}", width, height);
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

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    // A copy of the console output will automatically be saved to a logfile
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string(meta::APP_NAME) + ".log", true);
    auto logger = std::make_shared<spdlog::async_logger>("main", spdlog::sinks_init_list{console_sink, file_sink},
                                                         spdlog::thread_pool(), spdlog::async_overflow_policy::block);

    logger->flush_on(spdlog::level::trace);
    logger->set_level(spdlog::level::trace);
    logger->set_pattern("%Y-%m-%d %T.%f %^%l%$ %5t [%n] %v");

    // We only use one global logger by default, not one logger for each component of the code.
    spdlog::set_default_logger(logger);

    spdlog::trace("Inexor vulkan-renderer, BUILD " + std::string(__DATE__) + ", " + __TIME__);
}

ExampleApp::ExampleApp(int argc, char **argv) {
    initialize_spdlog();

    using namespace vulkan_renderer::meta;

    spdlog::trace("Application version: {}", APP_VERSION_STR);
    spdlog::trace("Engine version: {}", ENGINE_VERSION_STR);

    // Parse command line arguments.
    CLI::App app{"vulkan-renderer"};
    argv = app.ensure_utf8(argv);
    app.add_flag("--vsync", m_vsync_enabled);
    std::optional<std::uint32_t> preferred_gpu;
    app.add_option("--gpu", preferred_gpu);
    std::uint32_t max_fps = FPSLimiter::DEFAULT_FPS;
    app.add_option("--maxfps", max_fps);
    app.parse(argc, argv);

    m_fps_limiter.set_max_fps(max_fps);

    load_toml_configuration_file("assets/configuration/renderer.toml");

    spdlog::trace("Creating Vulkan instance");

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
    if (wrapper::is_instance_extension_supported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
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
        if (!wrapper::is_instance_extension_supported(glfw_extensions[index])) {
            // If any of the instance extensions that is required by glfw is not available, we will fail.
            throw InexorException("Error: glfw instance extension '" + std::string(glfw_extensions[index]) +
                                  "' is not available on the system!");
        } else {
            spdlog::trace("   - {}", glfw_extensions[index]);
            instance_extensions.push_back(glfw_extensions[index]);
        }
    }

    if (wrapper::is_instance_layer_supported("VK_LAYER_KHRONOS_validation")) {
        instance_layers.push_back("VK_LAYER_KHRONOS_validation");
    } else {
        spdlog::error("Instance layer 'VK_LAYER_KHRONOS_validation' is not available on this system!");
    }

    m_instance = std::make_unique<Instance>(instance_layers, instance_extensions);

    m_dbg_callback = std::make_unique<VulkanDebugUtilsCallback>(*m_instance, validation_layer_debug_messenger_callback);

    m_input = std::make_unique<Input>();

    m_surface = std::make_unique<WindowSurface>(m_instance->instance(), m_window->window());

    setup_window_and_input_callbacks();

    spdlog::trace("Creating window surface");

    if (preferred_gpu) {
        spdlog::trace("Preferential graphics card index {} specified", *preferred_gpu);
    }

    if (m_vsync_enabled) {
        spdlog::trace("V-sync enabled!");
    } else {
        spdlog::trace("V-sync disabled!");
    }

    const auto physical_devices = tools::get_physical_devices(m_instance->instance());
    if (preferred_gpu && *preferred_gpu >= physical_devices.size()) {
        spdlog::critical("GPU index {} is out of range!", *preferred_gpu);
        // The most suitable gpu will be chosen automatically later.
        preferred_gpu = std::nullopt;
    }

    const VkPhysicalDeviceFeatures required_features{
        // Add required physical device features here
    };

    std::vector<const char *> required_extensions{
        // Since we want to draw on a window, we need the swapchain extension
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    const VkPhysicalDevice physical_device =
        preferred_gpu ? physical_devices[*preferred_gpu]
                      : tools::pick_best_physical_device(*m_instance, m_surface->surface(), required_features,
                                                         required_extensions);

    m_device = std::make_unique<Device>(*m_instance, m_surface->surface(), physical_device, required_features,
                                        required_extensions);

    m_swapchain = std::make_unique<Swapchain>(*m_device, "Default Swapchain", m_surface->surface(), m_window->width(),
                                              m_window->height(), m_vsync_enabled);

    m_swapchain2 = std::make_shared<Swapchain>(*m_device, "Default Swapchain", m_surface->surface(), m_window->width(),
                                               m_window->height(), m_vsync_enabled);

    m_camera = std::make_unique<Camera>(glm::vec3(6.0f, 10.0f, 2.0f), 180.0f, 0.0f,
                                        static_cast<float>(m_window->width()), static_cast<float>(m_window->height()));
    m_camera->set_movement_speed(5.0f);
    m_camera->set_rotation_speed(0.5f);

    load_shaders();

    m_uniform_buffers.emplace_back(*m_device, "matrices uniform buffer", sizeof(UniformBufferObject));

    // Create an instance of the resource descriptor builder.
    // This allows us to make resource descriptors with the help of a builder pattern.
    DescriptorBuilder descriptor_builder(*m_device);

    // Make use of the builder to create a resource descriptor for the uniform buffer.
    m_descriptors.emplace_back(
        descriptor_builder.add_uniform_buffer<UniformBufferObject>(m_uniform_buffers[0].buffer(), 0)
            .build("Default uniform buffer"));

    load_octree_geometry(true);
    generate_octree_indices();

    m_window->show();

    // RENDERGRAPH2
    m_pipeline_cache2 = std::make_unique<PipelineCache>(*m_device);

    // RENDERGRAPH2
    m_render_graph2 = std::make_unique<vulkan_renderer::render_graph::RenderGraph>(*m_device, *m_pipeline_cache2);

    recreate_swapchain();
}

void ExampleApp::render_frame() {
    if (m_window_resized) {
        m_window_resized = false;
        recreate_swapchain();
        return;
    }

    const auto image_index = m_swapchain->acquire_next_image_index();

    const auto &cmd_buf = m_device->request_command_buffer(VK_QUEUE_GRAPHICS_BIT, "rendergraph");

    m_render_graph->render(image_index, cmd_buf);

    const std::array<VkPipelineStageFlags, 1> stage_mask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    cmd_buf.submit_and_wait(inexor::vulkan_renderer::wrapper::make_info<VkSubmitInfo>({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_swapchain->image_available_semaphore_pointer(),
        .pWaitDstStageMask = stage_mask.data(),
        .commandBufferCount = 1,
    }));

    m_swapchain->present(image_index);

    // RENDERGRAPH2
    // @TODO Abstract this into rendergraph!
    const auto img_index2 = m_swapchain2->acquire_next_image_index();
    const auto &cmd_buf2 = m_device->request_command_buffer(VK_QUEUE_GRAPHICS_BIT, "rendergraph2");
    m_render_graph2->render();
    cmd_buf.submit_and_wait(inexor::vulkan_renderer::wrapper::make_info<VkSubmitInfo>({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_swapchain2->image_available_semaphore_pointer(),
        .pWaitDstStageMask = stage_mask.data(),
        .commandBufferCount = 1,
    }));
    m_swapchain2->present(img_index2);

    if (auto fps_value = m_fps_limiter.get_fps()) {
        m_window->set_title("Inexor Vulkan API renderer demo - " + std::to_string(*fps_value) + " FPS");
        spdlog::trace("FPS: {}, window size: {} x {}", *fps_value, m_window->width(), m_window->height());
    }
}

void ExampleApp::recreate_swapchain() {
    m_window->wait_for_focus();
    m_device->wait_idle();

    // Query the framebuffer size here again although the window width is set during framebuffer resize callback
    // The reason for this is that the framebuffer size could already be different again because we missed a poll
    // This seems to be an issue on Linux only though
    auto [window_width, window_height] = m_window->get_framebuffer_size();

    // TODO: This should be abstracted itno a method of the Window wrapper.
    // TODO: This is quite naive, we don't need to recompile the whole render graph on swapchain invalidation.
    m_render_graph.reset();
    // Recreate the swapchain
    m_swapchain->setup_swapchain(
        VkExtent2D{static_cast<std::uint32_t>(window_width), static_cast<std::uint32_t>(window_height)},
        m_vsync_enabled);

    // RENDERGRAPH2
    // Recreate the swapchain
    m_swapchain2->setup_swapchain(
        VkExtent2D{static_cast<std::uint32_t>(window_width), static_cast<std::uint32_t>(window_height)},
        m_vsync_enabled);

    m_render_graph = std::make_unique<RenderGraph>(*m_device, *m_swapchain);
    // RENDERGRAPH2
    m_render_graph2 =
        std::make_unique<inexor::vulkan_renderer::render_graph::RenderGraph>(*m_device, *m_pipeline_cache2);

    setup_render_graph();

    m_camera->set_aspect_ratio(window_width, window_height);

    m_imgui_overlay.reset();

    // RENDERGRAPH2
    m_imgui_overlay = std::make_unique<ImGUIOverlay>(*m_device, *m_swapchain, m_swapchain2, m_render_graph.get(),
                                                     m_back_buffer, m_graphics_pass2, m_render_graph2, [&]() {
                                                         // RENDERGRAPH2
                                                         update_imgui_overlay();
                                                     });
    m_render_graph->compile(m_back_buffer);

    // RENDERGRAPH2
    m_render_graph2->compile();
}

void ExampleApp::setup_render_graph() {
    // RENDERGRAPH2
    // @TODO Where to place this? DO we need this here?
    m_render_graph2->reset();

    m_back_buffer = m_render_graph->add<TextureResource>("back buffer", TextureUsage::BACK_BUFFER);
    m_back_buffer->set_format(m_swapchain->image_format());
    // RENDERGRAPH2
    m_back_buffer2 = m_render_graph2->add_texture(
        "back buffer", vulkan_renderer::render_graph::TextureUsage::COLOR_ATTACHMENT, m_swapchain->image_format(),
        m_swapchain->extent().width, m_swapchain->extent().height, 1, VK_SAMPLE_COUNT_1_BIT, [&]() {
            //
        });

    auto *depth_buffer = m_render_graph->add<TextureResource>("depth buffer", TextureUsage::DEPTH_STENCIL_BUFFER);
    depth_buffer->set_format(VK_FORMAT_D32_SFLOAT_S8_UINT);
    // RENDERGRAPH2
    m_depth_buffer2 = m_render_graph2->add_texture(
        "depth buffer", vulkan_renderer::render_graph::TextureUsage::DEPTH_ATTACHMENT, VK_FORMAT_D32_SFLOAT_S8_UINT,
        m_swapchain->extent().width, m_swapchain->extent().height, 1, VK_SAMPLE_COUNT_1_BIT, [&]() {
            //
        });

    m_index_buffer = m_render_graph->add<BufferResource>("index buffer", BufferUsage::INDEX_BUFFER);
    m_index_buffer->upload_data(m_octree_indices);
    // RENDERGRAPH2
    m_index_buffer2 =
        m_render_graph2->add_buffer("index buffer", vulkan_renderer::render_graph::BufferType::INDEX_BUFFER, [&]() {
            // Request rendergraph to update the index buffer
            m_index_buffer2.lock()->request_update(m_octree_indices);
        });

    m_vertex_buffer = m_render_graph->add<BufferResource>("vertex buffer", BufferUsage::VERTEX_BUFFER);
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, position)); // NOLINT
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(OctreeGpuVertex, color));    // NOLINT
    m_vertex_buffer->upload_data(m_octree_vertices);
    // RENDERGRAPH2
    m_vertex_buffer2 =
        m_render_graph2->add_buffer("vertex buffer", vulkan_renderer::render_graph::BufferType::VERTEX_BUFFER, [&]() {
            // Request rendergraph to update the vertex buffer
            m_vertex_buffer2.lock()->request_update(m_octree_vertices);
        });

    // RENDERGRAPH2
    m_graphics_pass2 = m_render_graph2->get_graphics_pass_builder()
                           .writes_to(m_back_buffer2)
                           .writes_to(m_depth_buffer2)
                           .set_on_record([&](const CommandBuffer &cmd_buf) {
                               cmd_buf.bind_descriptor_set(m_descriptor_set2, m_octree_pipeline2)
                                   .bind_vertex_buffer(m_vertex_buffer2)
                                   .bind_index_buffer(m_index_buffer2)
                                   .draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
                           })
                           .build("Octree", vulkan_renderer::wrapper::DebugLabelColor::GREEN);
    // RENDERGRAPH2
    // Descriptor management for the model/view/projection uniform buffer
    m_render_graph2->add_resource_descriptor(
        [&](vulkan_renderer::wrapper::descriptors::DescriptorSetLayoutBuilder &builder) {
            m_descriptor_set_layout2 = builder
                                           .add(vulkan_renderer::wrapper::descriptors::DescriptorType::UNIFORM_BUFFER,
                                                VK_SHADER_STAGE_VERTEX_BIT)
                                           .build("model/view/proj");
        },
        [&](vulkan_renderer::wrapper::descriptors::DescriptorSetAllocator &allocator) {
            m_descriptor_set2 = allocator.allocate("model/view/proj", m_descriptor_set_layout2);
        },
        [&](vulkan_renderer::wrapper::descriptors::WriteDescriptorSetBuilder &builder) {
            // TODO: Modify to create several descriptor sets (an array?) for each octree
            // TODO: Specify camera matrix as push constant
            // TODO: Multiply view and perspective matrix on cpu and pass as one matrix!
            // TODO: Use one big descriptor (array?) and pass view*perspective and array index as push constant!
            // This will require changes to DescriptorSetLayoutBuilder and more!
            return builder.add(m_descriptor_set2, m_mvp_matrix2, 0).build();
        });
    // RENDERGRAPH2
    m_mvp_matrix2 = m_render_graph2->add_buffer("model/view/proj",
                                                vulkan_renderer::render_graph::BufferType::UNIFORM_BUFFER, [&]() {
                                                    // TODO: Is there a way to avoid external code to call
                                                    // request_update? Should we restrict this to the lambda?
                                                    m_ubo.model = glm::mat4(1.0f);
                                                    m_ubo.view = m_camera->view_matrix();
                                                    m_ubo.proj = m_camera->perspective_matrix();
                                                    m_ubo.proj[1][1] *= -1;
                                                    // Request rendergraph to do an update of the uniform buffer
                                                    m_mvp_matrix2.lock()->request_update(m_ubo);
                                                });

    // @TODO We don't need to re-load the shaders when recreating swapchain!
    // RENDERGRAPH2
    m_vertex_shader2 =
        std::make_shared<Shader>(*m_device, VK_SHADER_STAGE_VERTEX_BIT, "Octree", "shaders/main.vert.spv");
    m_fragment_shader2 =
        std::make_shared<Shader>(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "Octree", "shaders/main.frag.spv");

    // RENDERGRAPH2
    m_render_graph2->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        // RENDERGRAPH2
        m_octree_pipeline2 = builder.add_shader(m_vertex_shader2)
                                 .add_shader(m_fragment_shader2)
                                 .set_vertex_input_bindings({{
                                     .binding = 0,
                                     .stride = sizeof(OctreeVertex),
                                     .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                                 }})
                                 .set_vertex_input_attributes({
                                     {
                                         .location = 0,
                                         .format = VK_FORMAT_R32G32B32_SFLOAT,
                                         .offset = offsetof(OctreeVertex, position),
                                     },
                                     {
                                         .location = 1,
                                         .format = VK_FORMAT_R32G32B32_SFLOAT,
                                         .offset = offsetof(OctreeVertex, color),
                                     },
                                 })
                                 .set_multisampling(VK_SAMPLE_COUNT_1_BIT)
                                 .add_default_color_blend_attachment()
                                 .set_depth_attachment_format(m_depth_buffer2.lock()->format())
                                 .add_color_attachment_format(m_back_buffer2.lock()->format())
                                 .set_viewport(m_back_buffer2.lock()->extent())
                                 .set_scissor(m_back_buffer2.lock()->extent())
                                 .set_descriptor_set_layout(m_descriptor_set_layout2)
                                 .build("Octree", true);
    });

    // RENDERGRAPH2
    m_graphics_pass2 = m_render_graph2->add_graphics_pass(
        // @TODO bind pipeline!
        m_render_graph2->get_graphics_pass_builder()
            .writes_to(m_back_buffer2)
            .writes_to(m_depth_buffer2)
            .set_on_record([&](const CommandBuffer &cmd_buf) {
                cmd_buf
                    .bind_pipeline(m_octree_pipeline2)
                    // @TODO Associate descriptor set with a pipeline layout?
                    .bind_descriptor_set(m_descriptor_set2, m_octree_pipeline2)
                    .draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
            })
            .build("Octree", vulkan_renderer::render_graph::DebugLabelColor::GREEN));

    auto *main_stage = m_render_graph->add<GraphicsStage>("main stage");
    main_stage->writes_to(m_back_buffer);
    main_stage->writes_to(depth_buffer);
    main_stage->reads_from(m_index_buffer);
    main_stage->reads_from(m_vertex_buffer);
    main_stage->bind_buffer(m_vertex_buffer, 0);
    main_stage->set_clears_screen(true);
    main_stage->set_depth_options(true, true);
    main_stage->set_on_record([&](const PhysicalStage &physical, const CommandBuffer &cmd_buf) {
        cmd_buf.bind_descriptor_sets(m_descriptors[0].descriptor_sets(), physical.m_pipeline->pipeline_layout());
        cmd_buf.draw_indexed(static_cast<std::uint32_t>(m_octree_indices.size()));
    });

    for (const auto &shader : m_shaders) {
        main_stage->uses_shader(shader);
    }

    main_stage->add_descriptor_layout(m_descriptors[0].descriptor_set_layout());
}

void ExampleApp::update_uniform_buffers() {
    m_ubo.model = glm::mat4(1.0f);
    m_ubo.view = m_camera->view_matrix();
    m_ubo.proj = m_camera->perspective_matrix();
    m_ubo.proj[1][1] *= -1;

    // TODO: Embed this into the render graph.
    m_uniform_buffers[0].update(&m_ubo, sizeof(m_ubo));
}

void ExampleApp::update_imgui_overlay() {
    auto cursor_pos = m_input->kbm_data().get_cursor_pos();

    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = m_time_passed;
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
    ImGui::Text("Vulkan API %d.%d.%d", VK_API_VERSION_MAJOR(VK_API_VERSION_1_2),
                VK_API_VERSION_MINOR(VK_API_VERSION_1_2), VK_API_VERSION_PATCH(VK_API_VERSION_1_2));
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
    ImGui::PushItemWidth(150.0f * m_imgui_overlay->scale());
    ImGui::PopItemWidth();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::Render();

    m_imgui_overlay->update();
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
    m_camera->update(m_time_passed);
    m_camera->set_movement_state(CameraMovement::FORWARD, m_input->kbm_data().is_key_pressed(GLFW_KEY_W));
    m_camera->set_movement_state(CameraMovement::LEFT, m_input->kbm_data().is_key_pressed(GLFW_KEY_A));
    m_camera->set_movement_state(CameraMovement::BACKWARD, m_input->kbm_data().is_key_pressed(GLFW_KEY_S));
    m_camera->set_movement_state(CameraMovement::RIGHT, m_input->kbm_data().is_key_pressed(GLFW_KEY_D));
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
            update_uniform_buffers();
            update_imgui_overlay();
            render_frame();
            process_input();
            if (m_input->kbm_data().was_key_pressed_once(GLFW_KEY_N)) {
                load_octree_geometry(false);
                generate_octree_indices();
                m_index_buffer->upload_data(m_octree_indices);
                m_vertex_buffer->upload_data(m_octree_vertices);
            }
            m_camera->update(m_time_passed);
            m_time_passed = m_stopwatch.time_step();
            check_octree_collisions();
        }
    }
}

} // namespace inexor::example_app
