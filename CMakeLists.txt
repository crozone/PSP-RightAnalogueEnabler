cmake_minimum_required(VERSION 3.11)

project(RightAnalogueEnabler)

add_prx_module(${PROJECT_NAME}
    right_analogue_enabler.c
    exports.exp
    imports.S
)

set_property(SOURCE imports.S PROPERTY LANGUAGE C)

target_compile_definitions(
    # If the debug configuration pass the DEBUG define to the compiler
    ${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pspmodinfo
    pspsdk
    pspkernel
)

target_link_options(${PROJECT_NAME} PRIVATE -nostdlib)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "RightAnalogueEnabler")
