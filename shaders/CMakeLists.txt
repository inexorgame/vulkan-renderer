find_program(GLSL_VALIDATOR glslangValidator REQUIRED)
if(NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found!")
endif()

set(
    SHADERS
    gltf/filtercube.vert
    gltf/genbrdflut.frag
    gltf/genbrdflut.vert
    gltf/irradiancecube.frag
    gltf/pbr.vert
    gltf/pbr_khr.frag
    gltf/prefilterenvmap.frag
    imgui/ui.vert
    imgui/ui.frag
    octree/main.frag
    octree/main.vert
)

foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    get_filename_component(FILE_PATH ${SHADER} DIRECTORY)
    set(SPIRV ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}/${FILE_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
    )

    list(APPEND SPIRV_FILES ${SPIRV})
endforeach()

add_custom_target(inexor-shaders DEPENDS ${SPIRV_FILES})
