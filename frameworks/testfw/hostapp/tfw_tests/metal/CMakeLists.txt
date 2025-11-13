cmake_minimum_required(VERSION 2.8)

project(tfw_metal)
message("TESTFW_LIBRARIES in metal ${TESTFW_LIBRARIES}")

include(../../../testfw.cmake)
add_library(tfw_metal metal_basic.mm)
set_target_properties(tfw_metal PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
target_link_libraries(tfw_metal "-weak_framework Metal")
register_testfw_module(tfw_metal "tfw_metal")
