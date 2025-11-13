cmake_minimum_required(VERSION 3.5)
project(gfxbench_data)

include(${CMAKE_CURRENT_LIST_DIR}/copy_data.cmake)
install(CODE "message(\"Installation complete.\")")
