#include "inexor/vulkan-renderer/inexor_application.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace inexor::vulkan_renderer;

Application renderer;

int main(int argc, char *argv[]) {
    spdlog::init_thread_pool(8192, 2);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("vulkan-renderer.log", true);
    auto vulkan_renderer_log = std::make_shared<spdlog::async_logger>("vulkan-renderer", spdlog::sinks_init_list{console_sink, file_sink},
                                                                      spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    vulkan_renderer_log->set_level(spdlog::level::trace);
    vulkan_renderer_log->set_pattern("%Y-%m-%d %T.%f %^%l%$ %5P %5t [%-10n] %v");
    vulkan_renderer_log->flush_on(spdlog::level::debug); // TODO: as long as we don't have a flush on crash

    spdlog::set_default_logger(vulkan_renderer_log);

    spdlog::debug("Inexor vulkan-renderer, BUILD " + std::string(__DATE__) + ", " + __TIME__);

    spdlog::debug("Parsing command line arguments.");

    // We use some simple command line argument parser we wrote ourselves.
    renderer.parse_command_line_arguments(argc, argv);

    VkResult result = renderer.init();

    if (VK_SUCCESS == result) {
        renderer.run();
        renderer.calculate_memory_budget();
        renderer.cleanup();

        spdlog::debug("Window closed.");
    } else {
        // Something did go wrong when initialising the engine!
        vulkan_error_check(result);
        return -1;
    }

    return 0;
}
