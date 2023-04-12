FetchContent_MakeAvailable(gtest)
FetchContent_MakeAvailable(benchmark)

set(INEXOR_BENCHMARKING_SOURCE_FILES
    engine_benchmark_main.cpp
    world/cube_collision.cpp
)

add_executable(inexor-vulkan-renderer-benchmarks ${INEXOR_BENCHMARKING_SOURCE_FILES})

set_target_properties(
    inexor-vulkan-renderer-benchmarks PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(
    inexor-vulkan-renderer-benchmarks

    PRIVATE
    inexor-vulkan-renderer

    PUBLIC
    benchmark::benchmark
)
