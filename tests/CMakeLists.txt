set(INEXOR_UNIT_TEST_SOURCE_FILES
    unit_tests_main.cpp

    world/cube_collision.cpp
)

add_executable(inexor-vulkan-renderer-tests ${INEXOR_UNIT_TEST_SOURCE_FILES})

set_target_properties(
    inexor-vulkan-renderer-tests PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(
    inexor-vulkan-renderer-tests

    PUBLIC
    # Include configured header file which contains engine metadata
    ${CMAKE_BINARY_DIR}/src/include/inexor/vulkan-renderer/
)

target_link_libraries(
    inexor-vulkan-renderer-tests

    PRIVATE
    inexor-vulkan-renderer
)
