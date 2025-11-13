cmake_minimum_required(VERSION 2.8)

project(tfw_schemas_test)

include(compiler_flags)

find_package(GTest REQUIRED)

list(APPEND TEST_SOURCE
    testrepositorytest.cpp
    apidefinitiontest.cpp
)

add_executable(tfw_schemas_test ${TEST_SOURCE})
include_directories(${INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${GTEST_INCLUDE_DIRS})
target_link_libraries(tfw_schemas_test
    tfw_schemas
    ${GTEST_BOTH_LIBRARIES})

add_test(NAME tfw_schemas_test COMMAND tfw_schemas_test)
enable_testing()
