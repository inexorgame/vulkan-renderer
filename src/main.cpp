#include "vulkan-renderer/InexorApplication.hpp"

#include <spdlog/spdlog.h>

#include <thread>

using namespace inexor::vulkan_renderer;


int main(int argc, char* argv[])
{
    InexorApplication renderer;
    
    spdlog::set_level(spdlog::level::debug);
    
    spdlog::set_pattern("[%t][%H:%M:%S.%e][%^%l%$] %v");
    
    spdlog::debug("Inexor vulkan-renderer, BUILD " + std::string(__DATE__) + ", " + __TIME__);

    spdlog::debug("Parsing command line arguments.");
    
    // Parse the command line arguments.
    renderer.parse_command_line_arguments(argc, argv);

    if(VK_SUCCESS == renderer.init())
    {
        renderer.run();
        renderer.cleanup();
        
        spdlog::debug("Window closed.");
    }

    return 0;
}
