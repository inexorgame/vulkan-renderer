add_executable(
    inexor-vulkan-renderer-example

    main.cpp
)

set_target_properties(
    inexor-vulkan-renderer-example PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(
    inexor-vulkan-renderer-example

    PRIVATE
    inexor-vulkan-renderer
)
