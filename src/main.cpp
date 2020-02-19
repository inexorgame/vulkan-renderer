#include "vulkan-renderer/InexorRenderer.hpp"

#include <thread>

using namespace inexor::vulkan_renderer;


int main(int argc, char* argv[])
{
    InexorRenderer renderer;

    // Parse the command line arguments.
    renderer.parse_command_line_arguments(argc, argv);


    if(VK_SUCCESS == renderer.init())
    {
        renderer.run();
        renderer.cleanup();
        
        cout << "Window closed." << endl;
    }

    std::cin.get();

    return 0;
}
