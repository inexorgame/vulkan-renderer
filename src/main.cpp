#include "vulkan-renderer/InexorRenderer.hpp"

#include <thread>

using namespace inexor::vulkan_renderer;


int main(int argc, char* argv[])
{
    InexorRenderer renderer;

    // TODO: Pass on command line arguments.

    renderer.init();
    renderer.run();
    renderer.cleanup();

    return 0;
}
