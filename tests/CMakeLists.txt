FetchContent_MakeAvailable(gtest)

set(INEXOR_UNIT_TEST_SOURCE_FILES
    unit_tests_main.cpp
    allocators/pool_allocator_tests.cpp
    gpu-selection/gpu_selection_tests.cpp
    queue-selection/queue_selection_tests.cpp
    swapchain/choose_settings_tests.cpp
    world/cube_collision_tests.cpp
    world/cube_tests.cpp
)

if(MSVC)
    add_executable(inexor-vulkan-renderer-tests
        ${INEXOR_UNIT_TEST_SOURCE_FILES}
        ${CMAKE_BINARY_DIR}/win_resource_file.rc
    )
else()
    add_executable(inexor-vulkan-renderer-tests
        ${INEXOR_UNIT_TEST_SOURCE_FILES}
    )
endif()

set_target_properties(
    inexor-vulkan-renderer-tests PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# enable warnings for our own code
target_compile_options(inexor-vulkan-renderer-tests PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(
    inexor-vulkan-renderer-tests

    PRIVATE
    inexor-vulkan-renderer-core-lib

    PUBLIC
    GTest::gtest
)

# suppress all compiler warnings for third-party dependencies
foreach(_dep gtest)
    target_compile_options(${_dep} PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w>
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
    )
endforeach()

if(MSVC)
    # Group dependencies in Visual Studio
    set(THIRD_PARTY_TARGETS
        gmock
        gmock_main
        gtest
        gtest_main
    )
    # Move all dependencies into a subfolder
    set_target_properties(${THIRD_PARTY_TARGETS}
        PROPERTIES
            EXCLUDE_FROM_ALL TRUE
            FOLDER "Dependencies"
    )
endif()
