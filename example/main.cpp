#include "inexor/vulkan-renderer/application.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[]) {
    spdlog::init_thread_pool(8192, 2);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("vulkan-renderer.log", true);
    auto vulkan_renderer_log =
        std::make_shared<spdlog::async_logger>("vulkan-renderer", spdlog::sinks_init_list{console_sink, file_sink},
                                               spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    vulkan_renderer_log->set_level(spdlog::level::trace);
    vulkan_renderer_log->set_pattern("%Y-%m-%d %T.%f %^%l%$ %5t [%-10n] %v");
    vulkan_renderer_log->flush_on(spdlog::level::debug); // TODO: as long as we don't have a flush on crash

    spdlog::set_default_logger(vulkan_renderer_log);

    spdlog::debug("Inexor vulkan-renderer, BUILD " + std::string(__DATE__) + ", " + __TIME__);
    spdlog::debug("Parsing command line arguments.");

    std::unique_ptr<inexor::vulkan_renderer::Application> renderer;

    try {
        renderer = std::make_unique<inexor::vulkan_renderer::Application>(argc, argv);
    } catch (const inexor::vulkan_renderer::VulkanException &exception) {
        spdlog::critical(exception.what());
        std::abort();
    }

    renderer->run();
    renderer->calculate_memory_budget();
    spdlog::debug("Window closed");
}
