add_executable(inexor-vulkan-renderer-tests
    unit_tests_main.cpp)

set_target_properties(inexor-vulkan-renderer-tests PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)

target_link_libraries(inexor-vulkan-renderer-tests PRIVATE
    inexor-vulkan-renderer)
