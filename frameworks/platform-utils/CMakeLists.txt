cmake_minimum_required(VERSION 3.5)
project(platform-utils)

if(NOT ANDROID)
  message(FATAL_ERROR "Platform utils currently only supports Android")
endif()

set(sources
  platform/platform_android.h
  platform_android.cpp
  battery.cpp
)

include_directories(${CMAKE_CURRENT_LIST_DIR})
add_library(platform_utils SHARED ${sources} ${headers})
target_link_libraries(platform_utils log android)
configure_file(${CMAKE_CURRENT_LIST_DIR}/platform_utils-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/platform_utils-config.cmake @ONLY)

install(DIRECTORY platform DESTINATION include)
install(TARGETS platform_utils DESTINATION lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/platform_utils-config.cmake DESTINATION lib/platform_utils)

add_custom_command(TARGET platform_utils POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:platform_utils> ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME}/$<TARGET_FILE_NAME:platform_utils>
  COMMAND ${CMAKE_STRIP} --strip-unneeded $<TARGET_FILE:platform_utils>)

if(ANDROID)
  install(TARGETS platform_utils DESTINATION ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME})
endif()
