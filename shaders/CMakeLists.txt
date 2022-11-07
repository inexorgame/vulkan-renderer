# TODO: Change on Conan 2.0
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(${is_multi_config} AND NOT DEFINED CMAKE_BUILD_TYPE)
    find_package(glslang CONFIG REQUIRED)
else()
    find_package(glslang REQUIRED)
endif()

find_program(GLSL_VALIDATOR glslangValidator REQUIRED)
if(NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found!")
endif()

set(
    SHADERS
    main.vert
    main.frag
    ui.frag
    ui.vert
)

foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
    )

    list(APPEND SPIRV_FILES ${SPIRV})
endforeach()

source_group("Shader Files" FILES ${SHADERS})

add_custom_target(inexor-shaders DEPENDS ${SPIRV_FILES} SOURCES ${SHADERS})
