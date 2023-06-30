cmake_minimum_required(VERSION 2.8.12)

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif ()

set(EXAMPLES
        mandelbrot
        test
        )

find_program(GLSL_VALIDATOR glslangValidator REQUIRED)

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "shader/*.comp"
        )

foreach (_glsl_file ${GLSL_SOURCE_FILES})
    get_filename_component(_glsl_name ${_glsl_file} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${_glsl_name}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND ${GLSL_VALIDATOR} -V ${_glsl_file} -o ${SPIRV}
            DEPENDS ${_glsl_file})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach ()

add_custom_target(
        shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

foreach (example ${EXAMPLES})
    file(GLOB SOURCE_FILES
            ${example}/*.cc
            ${example}/*.c)
    add_executable(${example} ${SOURCE_FILES})
    target_link_libraries(${example} ${PROJECT_NAME})
    add_dependencies(${example} shaders)
    add_custom_command(TARGET ${example} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${example}>/shaders/"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_BINARY_DIR}/shaders"
            "$<TARGET_FILE_DIR:${example}>/shaders"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_BINARY_DIR}/shaders"
            "${CMAKE_BINARY_DIR}/bin/shaders"
            )
endforeach ()

add_custom_target(examples ALL DEPENDS ${EXAMPLES})
