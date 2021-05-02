add_executable(
    inexor-vulkan-renderer-benchmarks
    engine_benchmark_main.cpp
)

set_target_properties(
    inexor-vulkan-renderer-benchmarks PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(
    inexor-vulkan-renderer-benchmarks

    PUBLIC
    # Include configured header file which contains engine metadata
    ${CMAKE_BINARY_DIR}/src/include/inexor/vulkan-renderer/
)

target_link_libraries(
    inexor-vulkan-renderer-benchmarks

    PRIVATE
    inexor-vulkan-renderer
)
