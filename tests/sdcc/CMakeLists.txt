find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
find_program(UCSIM_65C02_EXECUTABLE NAMES ucsim_mos6502)
if(NOT UCSIM_65C02_EXECUTABLE)
    message(FATAL_ERROR "SDCC tests require ucsim_mos6502 on PATH")
endif()

# The other geometry suites exceed the image or hardware stack limits;
# see NOTES.md. All geometry implementation modules are still compiled.
set(SDCC_TEST_GROUPS "Coord;Vector" CACHE STRING
    "Geometry test groups to build and run with SDCC")
foreach(group IN LISTS SDCC_TEST_GROUPS)
    add_executable(${group} runtime.c "../${group}Test.c")
    target_compile_definitions(${group} PRIVATE TEST_FUNCTION=${group}_tests)
    target_link_libraries(${group} PRIVATE 3dObj)
    target_include_directories(${group} PRIVATE "${cbutil_SOURCE_DIR}")
    target_link_options(${group} PRIVATE --code-loc 0x2000 --xram-loc 0x0200
        "-Wl-a GSINIT=0x2000")
    add_test(NAME ${group}
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/run.py"
            "${UCSIM_65C02_EXECUTABLE}" "$<TARGET_FILE:${group}>")
    set_tests_properties(${group} PROPERTIES TIMEOUT 65)
endforeach()
