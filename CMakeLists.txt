cmake_minimum_required( VERSION 3.7 )

project( Shooter )

find_program(GLSLANG_VALIDATOR NAMES glslangValidator)
if(NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found!")
endif()

function(compile_shaders)
    if(NOT ARGN)
        return()
    endif()
    foreach(SHADER ${ARGN})
        get_filename_component(FILE_NAME ${SHADER} NAME)
        get_filename_component(FILE_DIR ${SHADER} DIRECTORY)
        set(output_file ${CMAKE_CURRENT_BINARY_DIR}/res/${FILE_DIR}/${FILE_NAME}.spv)
        set(compiled_shaders ${compiled_shaders} ${output_file})
        set(compiled_shaders ${compiled_shaders} PARENT_SCOPE)
        message(STATUS "${CMAKE_SOURCE_DIR}")
        add_custom_command(
                OUTPUT ${output_file}
                COMMAND ${CMAKE_COMMAND} -E make_directory res/${FILE_DIR}
                #COMMAND mkdir -p shaders
                COMMAND ${GLSLANG_VALIDATOR} -w -V ${CMAKE_SOURCE_DIR}/${SHADER} -o ${output_file}
                DEPENDS ${CMAKE_SOURCE_DIR}/${SHADER}
                COMMENT "Compiling shader ${output_file}"
        )
    endforeach()
    add_custom_target(shaders ALL DEPENDS ${compiled_shaders})
    add_dependencies(${PROJECT_NAME} shaders)
endfunction()

add_subdirectory( external )
add_subdirectory( src )