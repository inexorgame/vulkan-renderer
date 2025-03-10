set(INEXOR_EXAMPLE_APP_NAME "Example-app")
set(INEXOR_EXAMPLE_APP_VERSION_MAJOR 0)
set(INEXOR_EXAMPLE_APP_VERSION_MINOR 1)
set(INEXOR_EXAMPLE_APP_VERSION_PATCH 0)

# Extract the current git sha
find_package(Git REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --abbrev=7
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE INEXOR_GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Replace variables in header file with CMake values
configure_file(
    ${PROJECT_SOURCE_DIR}/example-app/include/example_app_meta.hpp.in ${PROJECT_SOURCE_DIR}/example-app/include/example_app_meta.hpp
)

set(INEXOR_EXAMPLE_APP_SOURCE_FILES
    example_app.cpp
    example_app_base.cpp
    example_app_meta.cpp)

foreach(FILE ${INEXOR_EXAMPLE_APP_SOURCE_FILES})
    get_filename_component(PARENT_DIR "${FILE}" PATH)

    string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")

    if("${FILE}" MATCHES ".*\\.cpp")
        set(GROUP "Source Files\\${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.hpp")
        set(GROUP "Header Files\\${GROUP}")
    endif()

    source_group("${GROUP}" FILES "${FILE}")
endforeach()

add_executable(inexor-vulkan-renderer-example ${INEXOR_EXAMPLE_APP_SOURCE_FILES})

target_compile_features(inexor-vulkan-renderer-example PRIVATE cxx_std_20)
target_link_libraries(inexor-vulkan-renderer-example PRIVATE inexor-vulkan-renderer)

if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
    set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "inexor-vulkan-renderer-example")
    target_compile_options(inexor-vulkan-renderer-example PRIVATE "/MP")
    set_target_properties(
        inexor-vulkan-renderer-example
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    # Ignore MSVC linker warning LNK4099
    set_target_properties(inexor-vulkan-renderer-example PROPERTIES LINK_FLAGS "/ignore:4099")
    # Disable useless warnings
    target_compile_definitions(inexor-vulkan-renderer PRIVATE _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
endif()

if(MSVC)
    # enable exceptions when using MSVC toolchain, makes Clang on windows possible
    target_compile_options(inexor-vulkan-renderer-example PRIVATE "-EHs")
endif()
