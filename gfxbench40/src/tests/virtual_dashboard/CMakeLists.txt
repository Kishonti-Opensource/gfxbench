cmake_minimum_required(VERSION 3.5)
project(vdashboard)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../../frameworks/krl krl)
include_directories(${KRL_INCLUDE_DIRS})


set(SRC_HIGH_LEVEL_TESTS_3_0
	${CMAKE_CURRENT_LIST_DIR}/vdashboard.cpp
	${CMAKE_CURRENT_LIST_DIR}/virtual_dashboard.cpp

	${CMAKE_CURRENT_LIST_DIR}/../../es2_common/glb_scene_.cpp
	${CMAKE_CURRENT_LIST_DIR}/../../es2_common/glb_scene_.h

	${CMAKE_CURRENT_SOURCE_DIR}/../../es2_common/glb_mesh.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../es2_common/glb_mesh_opengl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vdb_material.h
	${CMAKE_CURRENT_SOURCE_DIR}/vdb_material.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vdb_material_opengl.cpp
	${SRC_KRL}
)

SOURCE_GROUP("" FILES ${SRC_HIGH_LEVEL_TESTS_3_0} )

include_directories(
	${CMAKE_CURRENT_LIST_DIR}/../../es2_common
	${CMAKE_CURRENT_SOURCE_DIR}/../../es2_common/
	${CMAKE_CURRENT_SOURCE_DIR}/../../common/
	${CMAKE_CURRENT_SOURCE_DIR}/../../
	${KCL_PATH_PUBLIC}/src
	${KCL_LIB_PATH_PUBLIC}/tinyxml/src
	${KCL_LIB_PATH_PUBLIC}/libz/src
	${CMAKE_CURRENT_SOURCE_DIR}/../../../../frameworks/testfw
)

set(target_name "virtual_dashboard")

add_library(${target_name}  ${SRC_HIGH_LEVEL_TESTS_3_0})
install(TARGETS ${target_name} DESTINATION lib)



set(LIBS
	${PLATFORM_DEPENDENT_LIBS}
	${APPLY_FIND_LIBRARIES}
	tfw_schemas
	${KCL_LIBRARIES}
	KRL
	glb_common
	${OGLX_LIBRARIES}
	${SDK_LIB}
	${ZLIB_LIBRARIES}
	${PNG_LIBRARIES}
)

if(ANDROID)
	list(APPEND LIBS -landroid)
endif()

if(ENABLE_FRAME_CAPTURE)
	list(append LIBS FRAME_CAPTURE)
endif()

target_link_libraries(${target_name} ${LIBS})
target_compile_definitions(${target_name} PRIVATE ${OGLX_DEFINITIONS})

add_definitions(-DNOT_USE_GFXB)

register_testfw_module( ${target_name} "virtual_dashboard")
install(TARGETS ${target_name} DESTINATION lib)
if(NOT "${TFW_PACKAGE_DIR}" STREQUAL "")
    install(TARGETS ${target_name}
            RUNTIME DESTINATION ${TFW_PACKAGE_DIR}/plugins
            LIBRARY DESTINATION ${TFW_PACKAGE_DIR}/plugins
            ARCHIVE DESTINATION ${TFW_PACKAGE_DIR}/plugins)
endif()


include("${CMAKE_CURRENT_LIST_DIR}/../../../setup_importlib_properties")
