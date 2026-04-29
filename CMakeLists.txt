cmake_minimum_required(VERSION 3.10)
project(3dObjLib C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
add_compile_options(-pedantic)

if(MSVC)
    # MSVC equivalent of -Wall -Wextra is /W4
    add_compile_options(/W4)
else()
    # For GCC and Clang
    add_compile_options(-Wall -Wextra)
endif()

file(GLOB SOURCES "*.c")
file(GLOB PUBLIC_HEADERS "*.h")

add_library(3dObj STATIC ${SOURCES} ${PUBLIC_HEADERS})

set_target_properties(3dObj PROPERTIES
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
    DEBUG_POSTFIX "dbg"
)

target_include_directories(3dObj PUBLIC .)
target_compile_definitions(3dObj PRIVATE $<$<CONFIG:Debug>:DEBUG_OUTPUT>)

install(TARGETS 3dObj
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
