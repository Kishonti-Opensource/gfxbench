cmake_minimum_required(VERSION 2.8)

project(tfw_tests)

include(apply_find_vars)
include(compiler_flags)
include(printvar)

find_package(epoxy REQUIRED)
find_package(ngrtl REQUIRED core)
apply_find_vars(NGRTL epoxy)
set(TESTFW_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

include(${TESTFW_ROOT}/testfw.cmake)

if (NOT TARGET tfw_schemas)
	add_subdirectory(${TESTFW_ROOT}/deviceinfo deviceinfo)
	add_subdirectory(${TESTFW_ROOT}/schemas schemas)
endif()

include_directories(
	${TESTFW_ROOT}
	${APPLY_FIND_VARS}
)

# Add the source groups to the project
add_library(${PROJECT_NAME} ${TESTFW_ROOT}/graphics/videostreamrenderer.cpp basic.cpp)
if (MSVC_VERSION GREATER 1600)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_DX)
endif()

target_link_libraries(${PROJECT_NAME} tfw_schemas tfw_deviceinfo ${APPLY_FIND_LIBRARIES})
register_testfw_module(${PROJECT_NAME} "tfw_basic")

install(TARGETS ${PROJECT_NAME} DESTINATION plugins)
install(DIRECTORY config DESTINATION .)

if (IOS)
	add_subdirectory(metal)
endif()
