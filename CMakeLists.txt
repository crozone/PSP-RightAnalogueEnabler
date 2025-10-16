cmake_minimum_required(VERSION 3.11)

project(RightAnalogueEnabler)

add_prx_module(${PROJECT_NAME}
    right_analogue_enabler.c
    exports.exp
    imports.S
)

set_property(SOURCE imports.S PROPERTY LANGUAGE C)
# set_target_properties(EmulatedControllerTest PROPERTIES USE_KERNEL_LIBS "1")

target_compile_definitions(
    # If the debug configuration pass the DEBUG define to the compiler
    ${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pspdebug
    pspdisplay
#    pspctrl
    pspge
)

set_target_properties(RightAnalogueEnabler PROPERTIES OUTPUT_NAME "right_analogue_enabler")
# target_include_directories(RightAnalogueEnabler PUBLIC ${CMAKE_SOURCE_DIR}/include)
