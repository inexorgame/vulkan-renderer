#include "InexorApplication.hpp"

using namespace inexor::vulkan_renderer;


int main(int argc, char* argv[])
{
    InexorApplication renderer;
    
    // Set the global spd log level to debug.
    // We can change this upon release to display only messages which have more important log levels (like errors).
    spdlog::set_level(spdlog::level::debug);
    
    spdlog::set_pattern("[%t][%H:%M:%S.%e][%^%l%$] %v");
    
    spdlog::debug("Inexor vulkan-renderer, BUILD " + std::string(__DATE__) + ", " + __TIME__);

    spdlog::debug("Parsing command line arguments.");
    
    renderer.parse_command_line_arguments(argc, argv);

    while(true)
    {
        VkResult result = renderer.init();

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
    }

    return 0;
}
