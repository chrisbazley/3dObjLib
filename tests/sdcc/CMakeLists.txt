find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
find_program(UCSIM_65C02_EXECUTABLE NAMES ucsim_mos6502)
if(NOT UCSIM_65C02_EXECUTABLE)
    message(FATAL_ERROR "SDCC tests require ucsim_mos6502 on PATH")
endif()

function(add_geometry_test name group)
    add_executable(${name} runtime.c "../${group}Test.c")
    target_compile_definitions(${name} PRIVATE TEST_FUNCTION=${group}_tests ${ARGN})
    target_link_libraries(${name} PRIVATE 3dObj)
    target_include_directories(${name} PRIVATE "${cbutil_SOURCE_DIR}")
    target_link_options(${name} PRIVATE --code-loc 0x2000 --xram-loc 0x0200
        "-Wl-a GSINIT=0x2000")
    add_test(NAME ${name}
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/run.py"
            "${UCSIM_65C02_EXECUTABLE}" "$<TARGET_FILE:${name}>")
    set_tests_properties(${name} PROPERTIES TIMEOUT 65)
endfunction()

set(SDCC_TEST_GROUPS "Coord;Vector" CACHE STRING
    "Complete geometry test groups to build and run with SDCC")
foreach(group IN LISTS SDCC_TEST_GROUPS)
    add_geometry_test(${group} ${group})
endforeach()

# Parts 2 and 3 still fail; keep separate programs available for diagnosis.
# Native builds continue to run the complete Vertex suite.
set(SDCC_VERTEX_TEST_PARTS "1" CACHE STRING
    "Vertex test parts to build and run with SDCC")
foreach(part IN LISTS SDCC_VERTEX_TEST_PARTS)
    if(NOT part MATCHES "^[1-3]$")
        message(FATAL_ERROR "Unknown Vertex test part: ${part}")
    endif()
    add_geometry_test(Vertex${part} Vertex VERTEX_TEST_PART=${part})
endforeach()
