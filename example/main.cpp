#include <spdlog/spdlog.h>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <inexor_application.hpp>

using namespace inexor::vulkan_renderer;

InexorApplication renderer;


int main(int argc, char* argv[])
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("vulkan-renderer-logfile.txt", true);

	spdlog::sinks_init_list sink_list = { file_sink, console_sink };

	spdlog::logger logger("vulkan-renderer", sink_list.begin(), sink_list.end());
	logger.set_level(spdlog::level::trace);

	auto vulkan_renderer_log = std::make_shared<spdlog::logger>("vulkan-renderer", spdlog::sinks_init_list({console_sink, file_sink}));

	vulkan_renderer_log->set_level(spdlog::level::trace);
	vulkan_renderer_log->set_pattern("%t %H:%M:%S.%f %^%l%$ %v");

	spdlog::set_default_logger(vulkan_renderer_log);

	spdlog::debug("Inexor vulkan-renderer, BUILD " + std::string(__DATE__) + ", " + __TIME__);

	spdlog::debug("Parsing command line arguments.");

	// We use some simple command line argument parser we wrote ourselves.
	renderer.parse_command_line_arguments(argc, argv);

	VkResult result = renderer.initialise();

	if(VK_SUCCESS == result)
	{
		renderer.run();
		renderer.calculate_memory_budget();
		renderer.cleanup();

		spdlog::debug("Window closed.");
	}
	else
	{
		// Something did go wrong when initialising the engine!
		vulkan_error_check(result);
		return -1;
	}

	return 0;
}
