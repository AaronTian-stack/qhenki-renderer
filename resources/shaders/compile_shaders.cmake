
function(compile_shaders SHADER_DIR OUTPUT_DIR)

    file(GLOB_RECURSE SHADER_SOURCES "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag" "${SHADER_DIR}/*.comp")

    foreach(SHADER_SOURCE ${SHADER_SOURCES})
        # Get the filename and extension
        get_filename_component(FILENAME ${SHADER_SOURCE} NAME_WE)
        get_filename_component(EXTENSION ${SHADER_SOURCE} EXT)


        # Compile the shader to SPIR-V
        set(SPIRV_OUTPUT "${OUTPUT_DIR}/${FILENAME}${EXTENSION}")
        add_custom_command(
                OUTPUT ${SPIRV_OUTPUT}
                COMMAND glslc ${SHADER_SOURCE} -o ${SPIRV_OUTPUT}
                MAIN_DEPENDENCY ${SHADER_SOURCE}
                COMMENT "Compiling ${SHADER_SOURCE}"
        )

        # Add the SPIR-V file as a source for the executable
        target_sources(${PROJECT_NAME} PRIVATE ${SPIRV_OUTPUT})
    endforeach()
endfunction()
