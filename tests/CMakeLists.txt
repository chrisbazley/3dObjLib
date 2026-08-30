file(GLOB SOURCES "*.c")
file(GLOB PRIVATE_HEADERS "*.h")

add_executable(3dObjTests ${SOURCES} ${PRIVATE_HEADERS})
target_link_libraries(3dObjTests PRIVATE 3dObj)

if(USE_OPTIONAL)
    target_link_libraries(3dObjTests PRIVATE CBUtil)
endif()

if(USE_FORTIFY)
    target_link_libraries(3dObjTests PRIVATE Fortify)
endif()

add_test(NAME "3dObjTests" COMMAND 3dObjTests)
