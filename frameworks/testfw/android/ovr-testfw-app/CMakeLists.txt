cmake_minimum_required(VERSION 2.8.12)

project(testfw_app_java)
include(mklink)

find_host_program(ANT_EXECUTABLE ant)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	set(CONFIG_TYPE release)
else()
	set(CONFIG_TYPE debug)
endif()

set(APK_PATH ${CMAKE_CURRENT_LIST_DIR}/bin/testfw_app-${CONFIG_TYPE}.apk)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/assets)
mklink("${CMAKE_CURRENT_LIST_DIR}/../../resultdetail" ${CMAKE_CURRENT_LIST_DIR}/assets/resultdetail)
add_custom_command(OUTPUT ${APK_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TFW_PACKAGE_DIR}/libs libs
    COMMAND ${ANT_EXECUTABLE} ${CONFIG_TYPE}
    DEPENDS ${TESTFW_MODULES} testfw_jni
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
add_custom_target(testfw_app_java ALL DEPENDS ${APK_PATH})
if("$ENV{ARCHIVE_ROOT}" STREQUAL "")
    install(FILES ${APK_PATH} DESTINATION ${TFW_PACKAGE_DIR}/apk)
else()
    install(FILES ${APK_PATH} DESTINATION $ENV{ARCHIVE_ROOT})
endif()
