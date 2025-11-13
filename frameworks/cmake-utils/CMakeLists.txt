cmake_minimum_required(VERSION 2.8)
project(cmake-utils)

configure_file(cmake-utils-config.cmake.in hide/cmake-utils-config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hide/cmake-utils-config.cmake
  DESTINATION share/cmake-utils)
install(DIRECTORY cmake doc scripts DESTINATION share/cmake-utils)

