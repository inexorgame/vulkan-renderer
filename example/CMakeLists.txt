add_executable(inexor-vulkan-renderer-example
main.cpp
# Windows resource file to set application icon and metadata
${CMAKE_BINARY_DIR}/include/inexor/vulkan-renderer/resource.hpp
${CMAKE_BINARY_DIR}/include/inexor/vulkan-renderer/win_resource_file.rc
)

set_target_properties(
    inexor-vulkan-renderer-example PROPERTIES

    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
    set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "inexor-vulkan-renderer-example")
    target_compile_options(inexor-vulkan-renderer-example PRIVATE "/MP")
    set_target_properties(
        inexor-vulkan-renderer-example
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
endif()

# enable exceptions when using MSVC toolchain, makes Clang on windows possible
if(MSVC)
    target_compile_options(inexor-vulkan-renderer-example PRIVATE "-EHs")
endif()

target_link_libraries(
    inexor-vulkan-renderer-example

    PRIVATE
    inexor-vulkan-renderer
)
