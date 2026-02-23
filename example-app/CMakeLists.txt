set(INEXOR_EXAMPLE_SOURCE_FILES
    main.cpp
    application.cpp
    renderer.cpp
)

if(MSVC)
    add_executable(inexor-vulkan-renderer-example
        ${INEXOR_EXAMPLE_SOURCE_FILES}
        ${CMAKE_BINARY_DIR}/win_resource_file.rc
    )
else()
    add_executable(inexor-vulkan-renderer-example
        ${INEXOR_EXAMPLE_SOURCE_FILES}
    )
endif()

target_compile_features(inexor-vulkan-renderer-example PRIVATE cxx_std_20)
target_link_libraries(inexor-vulkan-renderer-example PRIVATE inexor-vulkan-renderer-core-lib)

if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
    set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "inexor-vulkan-renderer-example")
    target_compile_options(inexor-vulkan-renderer-example PRIVATE "/MP")
    set_target_properties(
        inexor-vulkan-renderer-example
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    # Ignore MSVC linker warning LNK4099
    set_target_properties(inexor-vulkan-renderer-example PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

# enable exceptions when using MSVC toolchain, makes Clang on windows possible
if(MSVC)
    target_compile_options(inexor-vulkan-renderer-example PRIVATE "-EHs")
endif()

# Single-config generators
set_target_properties(inexor-vulkan-renderer-example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/example
)

# Multi-config generators (Visual Studio, etc.)
foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    set_target_properties(inexor-vulkan-renderer-example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/example
    )
endforeach()

# install

include(GNUInstallDirs)

install(
    TARGETS inexor-vulkan-renderer-example
    COMPONENT example
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/assets/
    COMPONENT example
    DESTINATION ${CMAKE_INSTALL_BINDIR}/assets
)
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/shaders/
    COMPONENT example
    DESTINATION ${CMAKE_INSTALL_BINDIR}/shaders
    FILES_MATCHING PATTERN "*.spv"
)
