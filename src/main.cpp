#include "vulkan-renderer/InexorRenderer.hpp"

#include <thread>

using namespace inexor::vulkan_renderer;


int main(int argc, char* argv[])
{
    InexorRenderer renderer;

    // TODO: Pass on command line arguments.
    // TODO: Let the user specify which graphics card to use by passing command line argument -GPU <number>.
    // TODO: Initialise an empty octree-world.

    renderer.init();
    renderer.run();
    renderer.cleanup();
    
    cout << "Window closed." << endl;

    std::cin.get();

    return 0;
}
