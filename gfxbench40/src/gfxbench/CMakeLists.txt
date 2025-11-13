include(apply_find_vars)
find_package(ngrtl REQUIRED core)
apply_find_vars(NGRTL)

include_directories(
    ${KCL_PATH_PUBLIC}/src
	${CMAKE_CURRENT_SOURCE_DIR}/../common
	${CMAKE_CURRENT_SOURCE_DIR}/../frame_player
	${CMAKE_CURRENT_LIST_DIR}/corporate
	${CMAKE_CURRENT_LIST_DIR}/community
	${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/testfw
	${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/testfw/schemas
	${CMAKE_CURRENT_LIST_DIR}/../es2_common
)

if(ENABLE_FRAME_CAPTURE)
	list(APPEND FRAMECAPTURE_SOURCES ../../framecapture/gl3_capture.cpp)
endif()

list(APPEND SHARED_GFXSOURCES ${CMAKE_CURRENT_LIST_DIR}/global_test_environment.cpp)
list(APPEND SHARED_GFXSOURCES ${CMAKE_CURRENT_LIST_DIR}/status_report.h)
list(APPEND SHARED_GFXSOURCES ${CMAKE_CURRENT_LIST_DIR}/status_report.cpp)

if (OPT_COMMUNITY_BUILD AND NOT OPT_WINSTORE)
	find_package(netman REQUIRED)
	apply_find_vars(NETMAN)
	list(APPEND SHARED_GFXSOURCES
		${CMAKE_CURRENT_LIST_DIR}/gfxbench.cpp 
		${CMAKE_CURRENT_LIST_DIR}/gfxbench_ios_rawconfig.cpp # it's an empty function if not on ios
		)
else()
	list(APPEND SHARED_GFXSOURCES
	            ${CMAKE_CURRENT_LIST_DIR}/gfxbench.cpp
	            ${CMAKE_CURRENT_LIST_DIR}/gfxbench_ios_rawconfig.cpp
	            ${FRAMEPLAYER_SOURCES}
	            ${FRAMECAPTURE_SOURCES})
endif()

if(ANDROID)
	list(APPEND ALL_LIBS -landroid)
endif()

if (UNIX AND NOT ANDROID AND NOT MACOSX AND NOT IOS AND NOT QNX)
	list(APPEND ALL_LIBS dl rt)
endif()

set(ALL_LIBS
	
	${KCL_LIBRARIES}
	${EXTERNAL_LIBS}
	${PLATFORM_DEPENDENT_LIBS}
	${APPLY_FIND_LIBRARIES}
	tfw_schemas
	${ZLIB_LIBRARIES}
	${PNG_LIBRARIES}
	${ALL_LIBS}
    )
