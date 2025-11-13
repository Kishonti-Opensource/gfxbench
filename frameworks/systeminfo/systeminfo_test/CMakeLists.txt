cmake_minimum_required(VERSION 3.5)

set(SOURCE ${HEADERS} ${SOURCE} ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp)

find_package(ngrtl REQUIRED COMPONENTS core)
list(APPEND LIBRARIES ${NGRTL_LIBRARIES})

include(${CMAKE_CURRENT_LIST_DIR}/../CMakeLists.txt)

add_executable(systeminfo_test ${HEADERS} ${SOURCE} ${PLATFORM_SOURCE} ${NULL_SOURCE})
target_link_libraries(systeminfo_test ${LIBRARIES} systeminfo)
