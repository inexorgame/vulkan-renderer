#include <benchmark/benchmark.h>

#include "vk_availability_checks.hpp"
using namespace inexor::vulkan_renderer::availability_checks;


static void BM_is_instance_layer_available(benchmark::State& state)
{
    for(auto _ : state)
    {
        is_instance_layer_available("VK_LAYER_LUNARG_standard_validation");
    }
}

BENCHMARK(BM_is_instance_layer_available);


static void BM_is_instance_extension_available(benchmark::State& state)
{
    for(auto _ : state)
    {
        is_instance_extension_available("VK_EXT_debug_utils");
    }
}

BENCHMARK(BM_is_instance_extension_available);
