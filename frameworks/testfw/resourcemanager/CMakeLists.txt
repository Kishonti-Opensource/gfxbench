cmake_minimum_required(VERSION 2.8)

project(tfw_deviceinfo)



include(compiler_flags)
include(apply_find_vars)



find_package(Poco REQUIRED COMPONENTS Foundation)
find_package(ngrtl REQUIRED core)

apply_find_vars(POCO NGRTL)

include_directories(${TESTFW_ROOT})



list(APPEND HEADERS
    checksum.h
    resourcemanager.h
)

list(APPEND SOURCE 
    checksum.cpp
    resourcemanager.cpp
)



add_library(tfw_resourcemanager STATIC ${HEADERS} ${SOURCE})
target_link_libraries(tfw_resourcemanager ${APPLY_FIND_LIBRARIES})


if(OPT_ENABLE_TESTS)
    add_executable(test_resourcemanager main.cpp)
    target_link_libraries(test_resourcemanager tfw_resourcemanager ${APPLY_FIND_LIBRARIES})
endif()
