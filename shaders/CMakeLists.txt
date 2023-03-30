find_package(glslang "1.3.236.0" REQUIRED)

message(STATUS ${CMAKE_PROGRAM_PATH})

find_program(GLSLANG_VALIDATOR "glslangValidator" REQUIRED)
if(NOT GLSLANG_VALIDATOR)
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
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
    )

    list(APPEND SPIRV_FILES ${SPIRV})
endforeach()

source_group("Shader Files" FILES ${SHADERS})

add_custom_target(inexor-shaders DEPENDS ${SPIRV_FILES} SOURCES ${SHADERS})
