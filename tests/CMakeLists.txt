FetchContent_MakeAvailable(gtest)

set(INEXOR_UNIT_TEST_SOURCE_FILES
    unit_tests_main.cpp
    gpu-selection/selection.cpp
    swapchain/choose_settings.cpp
    world/cube_collision.cpp
    world/cube.cpp
)

add_executable(inexor-vulkan-renderer-tests ${INEXOR_UNIT_TEST_SOURCE_FILES})

set_target_properties(
    inexor-vulkan-renderer-tests PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(
    inexor-vulkan-renderer-tests

    PRIVATE
    inexor-vulkan-renderer

    PUBLIC
    gtest
)
