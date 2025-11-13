cmake_minimum_required(VERSION 3.5)
project(clew)

set(CMAKE_DEBUG_POSTFIX _d)

if (NOT BUILD_SHARED_LIBS)
	add_definitions(-DCLEW_STATIC)
endif()

set(CLEW_SRC
	${CMAKE_CURRENT_LIST_DIR}/include/CL/clew_function_types.h
	${CMAKE_CURRENT_LIST_DIR}/include/CL/clew_function_defines.h
	${CMAKE_CURRENT_LIST_DIR}/include/CL/clew.h
	${CMAKE_CURRENT_LIST_DIR}/clew.cpp
)

include_directories(
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/khronos
)

add_library(clew STATIC ${CLEW_SRC})

if(UNIX AND NOT APPLE)
	target_link_libraries(clew dl)
endif(UNIX AND NOT APPLE)

configure_file(${CMAKE_CURRENT_LIST_DIR}/clewConfig.cmake.in ${CMAKE_BINARY_DIR}/clewConfig.cmake @ONLY)

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY khronos/ DESTINATION include)
install(TARGETS clew
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/clewConfig.cmake DESTINATION lib/clew)
