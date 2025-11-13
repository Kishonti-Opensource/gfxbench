cmake_minimum_required(VERSION 2.8)
project(glew)
set(VERSION "1.13.0")

set(CMAKE_DEBUG_POSTFIX _d)


add_definitions(-DGLEW_NO_GLU)

if (CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fno-common -fPIC -Wall")
endif()

if (BUILD_SHARED_LIBS)
	add_definitions(-DGLEW_BUILD)
	find_package(OpenGL REQUIRED)
else()
	add_definitions(-DGLEW_STATIC)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
add_library(GLEW src/glew.c)
set_target_properties(GLEW PROPERTIES VERSION ${VERSION})
if (BUILD_SHARED_LIBS)
	target_link_libraries(GLEW ${OPENGL_gl_LIBRARY})
	set_target_properties(GLEW PROPERTIES RUNTIME_OUTPUT_NAME glew32)
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/GLEW-config.cmake.in ${CMAKE_BINARY_DIR}/glew-config.cmake @ONLY)

install(DIRECTORY include/ DESTINATION include)
install(TARGETS GLEW DESTINATION lib RUNTIME DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/glew-config.cmake DESTINATION lib/GLEW)
