cmake_minimum_required(VERSION 2.8.12)

project(benchmark_service)


if( "$ENV{DISABLE_NETMAN}" STREQUAL "1" )
add_definitions(-DDISABLE_NETMAN)
endif()

set(TESTFW_ROOT ${CMAKE_CURRENT_LIST_DIR}/../testfw)
if (NOT TARGET tfw_deviceinfo)
    add_subdirectory(${TESTFW_ROOT}/deviceinfo ${CMAKE_CURRENT_BINARY_DIR}/tfw_deviceinfo)
endif()
if (NOT TARGET tfw_schemas)
    add_subdirectory(${TESTFW_ROOT}/schemas ${CMAKE_CURRENT_BINARY_DIR}/tfw_schemas)
endif()



include(apply_find_vars)
include(compiler_flags)
include(CMakePackageConfigHelpers)
include(EmbedFiles)
include(GenerateExportHeader)

function(replace_in_file FILENAME REGEXP REPLACEMENT)
    file(READ "${FILENAME}" ORIGINAL_TEXT)
    string(REGEX REPLACE "${REGEXP}" "${REPLACEMENT}" NEW_TEXT "${ORIGINAL_TEXT}")
    file(WRITE "${FILENAME}" "${NEW_TEXT}")
endfunction()



find_package(clew QUIET)
if(clew_FOUND)
  apply_find_vars(CLEW)
endif()

find_package(ngrtl REQUIRED core)
find_package(sysinf REQUIRED)
find_package(Poco REQUIRED XML Foundation)

find_package(Vulkan)
if(VULKAN_FOUND)
    apply_find_vars(VULKAN)
endif()
apply_find_vars(NGRTL SYSINF POCO)

if(OPT_COMMUNITY_BUILD)
    find_package(OpenSSL REQUIRED)
    find_package(netman REQUIRED)
    find_package(syncdir REQUIRED)
    apply_find_vars(OPENSSL NETMAN SYNCDIR)
endif()



file(GLOB CONFIGS "${TFW_PACKAGE_DIR}/config/*")
list(APPEND RESOURCES ${CONFIGS})
file(GLOB LANGUAGES "${TFW_PACKAGE_DIR}/localization/*")
list(APPEND RESOURCES ${LANGUAGES})
file(GLOB TESTLISTS "${TFW_PACKAGE_DIR}/testlist*.json")
if(EXISTS "${TFW_PACKAGE_DIR}/benchmark_tests.json")
    list(APPEND TESTLISTS "${TFW_PACKAGE_DIR}/benchmark_tests.json")
endif()
list(APPEND RESOURCES ${TESTLISTS})
embed_files(getResource RELATIVE "${TFW_PACKAGE_DIR}" FILES ${RESOURCES})



include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/../testfw
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SYSINF_INCLUDE_DIRS}
    ${JNI_INCLUDE_DIRS}
)



list(APPEND PUBLIC_HEADERS
    src/benchmarkservice.h
    src/cursor.h
    ${TESTFW_ROOT}/graphics/graphicscontext.h
    ${TESTFW_ROOT}/schemas/apidefinition.h
    ${TESTFW_ROOT}/schemas/graphics.h
    ${TESTFW_ROOT}/schemas/serializable.h
    ${TESTFW_ROOT}/deviceinfo/runtimeinfo.h
)

list(APPEND HEADERS
    src/applicationconfig.h
    src/backgroundtasks.h
    src/benchmarkexception.h
    src/chartcursor.h
    src/compareresult.h
    src/configuration.h
    src/corporatedatagateway.h
    src/datagateway.h
    src/dictionary.h
    src/duelitem.h
    src/executor.h
    src/listcursor.h
    src/networkgateway.h
    src/resultcursor.h
    src/resultitem.h
    src/session.h
    src/sqlite3.h
    src/systeminfoitem.h
    src/systeminfogateway.h
    src/testinfo.h
    src/testitem.h
    src/testrepository.h
    src/testrunner.h
    src/variant.h
    ${CMAKE_CURRENT_BINARY_DIR}/getResource.h
)

list(APPEND SOURCE
    src/applicationconfig.cpp
    src/backgroundtasks.cpp
    src/benchmarkexception.cpp
    src/benchmarkservice.cpp
    src/chartcursor.cpp
    src/compareresult.cpp
    src/configuration.cpp
    src/corporatedatagateway.cpp
    src/datagateway.cpp
    src/dictionary.cpp
    src/duelitem.cpp
    src/executor.cpp
    src/resultcursor.cpp
    src/resultitem.cpp
    src/session.cpp
    src/systeminfogateway.cpp
    src/systeminfoitem.cpp
    src/testinfo.cpp
    src/testitem.cpp
    src/testrepository.cpp
    src/testrunner.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/getResource.cpp
)

if(OPT_COMMUNITY_BUILD)
    add_definitions(-DSQLITE_HAS_CODEC=1 -DSQLITE_TEMP_STORE=2)
    list(APPEND HEADERS
        src/communitydatagateway.h
        src/communitynetworkgateway.h)
    list(APPEND SOURCE
        src/communitydatagateway.cpp
        src/communitynetworkgateway.cpp
        src/sqlcipher.c)
    set_source_files_properties(src/benchmarkservice.cpp PROPERTIES COMPILE_DEFINITIONS IS_COMMUNITY)
    set_source_files_properties(src/systeminfogateway.cpp PROPERTIES COMPILE_DEFINITIONS IS_COMMUNITY)
    set_source_files_properties(src/sqlcipher.c PROPERTIES COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
else()
    list(APPEND HEADERS
        src/corporatedatagateway.h)
    list(APPEND SOURCE
        src/corporatedatagateway.cpp
        src/sqlite3.c)
    set_source_files_properties(src/sqlite3.c PROPERTIES COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
endif()

list(APPEND TESTS unittests/testrepositorytest.cpp)

# Clean swig directories
if(OPT_SWIG_JAVA)
    file(GLOB SWIG_FILES_IN_BUILD "${CMAKE_CURRENT_BINARY_DIR}/java/*")
    file(GLOB SWIG_FILES_IN_INSTALL "${CMAKE_INSTALL_PREFIX}/swig/*")
    file(REMOVE ${SWIG_FILES_IN_BUILD} ${SWIG_FILES_IN_INSTALL})

    if (ANDROID)
        if(COMMAND find_host_package)
            find_host_package(SWIG REQUIRED)
        else()
            find_package(SWIG REQUIRED)
        endif()
        set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_LIST_DIR}/android/testfw-lib/src/net/kishonti/swig)
    else()
        set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_LIST_DIR}/java/net/kishonti/swig)
        find_package(SWIG REQUIRED)
        find_package(JNI REQUIRED)
        include_directories(${JNI_INCLUDE_DIRS})
    endif()

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/java/net/kishonti/benchmarkservice)
    execute_process(COMMAND ${SWIG_EXECUTABLE} -c++ -java
        -package net.kishonti.benchmarkservice
        -outdir ${CMAKE_CURRENT_BINARY_DIR}/java/net/kishonti/benchmarkservice
        -o ${CMAKE_CURRENT_BINARY_DIR}/benchmarkservice_wrap.cpp
        ${CMAKE_CURRENT_LIST_DIR}/benchmarkservice.i)
    list(APPEND SOURCE ${CMAKE_CURRENT_BINARY_DIR}/benchmarkservice_wrap.cpp)
    set(CURSOR_JAVA ${CMAKE_CURRENT_BINARY_DIR}/java/net/kishonti/benchmarkservice/Cursor.java)
    replace_in_file(${CURSOR_JAVA} "swigCPtr = cPtr;" "swigCPtr = cPtr;\n    setCallback(new Callback());")
endif()

if(OPT_SWIG_CSHARP)
    set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/cs)
    swig_add_module(benchmarkservice_cs csharp benchmarkservice.i)
    set_target_properties(benchmarkservice_cs PROPERTIES DEBUG_POSTFIX "")
    swig_link_libraries(benchmarkservice_cs benchmarkservice)
endif()

if(OPT_SWIG_JAVA)
    add_library(benchmarkservice SHARED ${PUBLIC_HEADERS} ${HEADERS} ${SOURCE})
else()
    add_library(benchmarkservice STATIC ${PUBLIC_HEADERS} ${HEADERS} ${SOURCE})
endif()

if(NOT WIN32)
    target_compile_options(benchmarkservice PRIVATE -Wno-deprecated-declarations)
endif()

set_target_properties(benchmarkservice PROPERTIES DEBUG_POSTFIX _d)
target_link_libraries(benchmarkservice
    PRIVATE tfw_deviceinfo
    PRIVATE tfw_schemas
    PRIVATE ${SYSINF_LIBRARIES}
    PRIVATE ${APPLY_FIND_LIBRARIES})
generate_export_header(benchmarkservice EXPORT_MACRO_NAME "BENCHMARK_SERVICE_EXPORT")
set(API_MACRO "#define BENCHMARK_SERVICE_EXPORT_H

#ifdef _WIN32
    #define BENCHMARK_SERVICE_API __stdcall
#else
    #define BENCHMARK_SERVICE_API
#endif
")
replace_in_file(${CMAKE_CURRENT_BINARY_DIR}/benchmarkservice_export.h "#define BENCHMARK_SERVICE_EXPORT_H" ${API_MACRO})
list(APPEND PUBLIC_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/benchmarkservice_export.h)

if(OPT_SWIG_JAVA)
    file(GLOB SWIG_FILES "${CMAKE_CURRENT_BINARY_DIR}/java/*")
    install(DIRECTORY ${SWIG_FILES} DESTINATION swig/)

    install(TARGETS benchmarkservice EXPORT benchmarkserviceTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION lib
        INCLUDES DESTINATION include
    )
    install(FILES ${PUBLIC_HEADERS} DESTINATION include/benchmarkservice)
    export(EXPORT benchmarkserviceTargets FILE ${CMAKE_CURRENT_BINARY_DIR}/benchmarkserviceTargets.cmake)
    install(EXPORT benchmarkserviceTargets FILE benchmarkserviceTargets.cmake DESTINATION share/benchmarkservice/cmake)
    install(FILES benchmarkserviceConfig.cmake DESTINATION share/benchmarkservice/cmake)
endif()


# # Unit tests
# if (0)
# find_package(GTest REQUIRED)
# apply_find_vars(GTEST)

# add_executable(benchmarkservicetest ${TESTS})

# target_link_libraries(benchmarkservicetest
#     benchmarkservice
#     tfw_schemas
#     ${GTEST_BOTH_LIBRARIES}
# )

# add_test(NAME benchmarkservicetest COMMAND benchmarkservicetest)
# enable_testing()
# endif()
