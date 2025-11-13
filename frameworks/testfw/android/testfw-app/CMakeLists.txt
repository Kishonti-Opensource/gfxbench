cmake_minimum_required(VERSION 3.5)

project(testfw_app_java)
include(mklink)

# Set up gradle
set(GRADLE_EXECUTABLE ${CMAKE_CURRENT_LIST_DIR}/gradlew)
execute_process(COMMAND chmod +x ${GRADLE_EXECUTABLE})

# Create the apk
set(APK_PATH ${CMAKE_CURRENT_LIST_DIR}/build/outputs/apk/release/testfw_app-release.apk)
add_custom_command(OUTPUT testfw_app_java_apk
    DEPENDS ${TESTFW_MODULES} testfw_jni
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TFW_PACKAGE_DIR}/libs ${CMAKE_CURRENT_LIST_DIR}/src/main/jnilibs
    COMMAND ${GRADLE_EXECUTABLE} -p ${CMAKE_CURRENT_LIST_DIR} assembleRelease
)

add_custom_target(testfw_app_java ALL DEPENDS testfw_app_java_apk)
if("$ENV{ARCHIVE_ROOT}" STREQUAL "")
    install(FILES ${APK_PATH} DESTINATION ${TFW_PACKAGE_DIR}/apk)
else()
    install(FILES ${APK_PATH} DESTINATION $ENV{ARCHIVE_ROOT})
endif()
