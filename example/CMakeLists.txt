add_executable(inexor-vulkan-renderer-example main.cpp)

set_target_properties(
    inexor-vulkan-renderer-example PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(inexor-vulkan-renderer-example PRIVATE "/MP")
    set_property(
        TARGET inexor-vulkan-renderer-example
        PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
endif()

target_link_libraries(
    inexor-vulkan-renderer-example

    PRIVATE
    inexor-vulkan-renderer
)
