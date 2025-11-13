cmake_minimum_required(VERSION 3.5)
project(tfw_schemas)

find_package(ngrtl REQUIRED core)
include(apply_find_vars)

apply_find_vars(NGRTL)
include(compiler_flags)

list(APPEND SOURCE
    apidefinition.h
    apidefinition.cpp
    descriptors.h
    descriptors.cpp
    compareitem.h
    compareitem.cpp
    compareresult.h
    compareresult.cpp
    configuration.h
    configuration.cpp
    compositedescriptor.h
    compositedescriptor.cpp
    resultitem.h
    resultitem.cpp
    serializable.h
    session.h
    testinfo.h
    testinfo.cpp
    testitem.h
    testitem.cpp
    testrepository.h
    testrepository.cpp
    chart.h
    chart.cpp
    computeresult.h
    computeresult.cpp
    gfxresult.h
    gfxresult.cpp
    result.h
    result.cpp
    graphics.h
    graphics.cpp
    compute.h
    compute.cpp
    environment.h
    environment.cpp
)

add_library(tfw_schemas STATIC ${SOURCE})
target_link_libraries(tfw_schemas ${APPLY_FIND_LIBRARIES})

if(BUILD_TESTING)
    add_subdirectory(unittests)
endif()
