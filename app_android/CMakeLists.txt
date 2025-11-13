cmake_minimum_required(VERSION 3.5)
project(app_android)

include(CheckRequiredVars)
include(VersionParse)
include(mklink)
include(printvar)

check_required_vars(PRODUCT_ID PRODUCT_VERSION PRODUCT_NAME TFW_PACKAGE_DIR)
version_parse(BENCHMARK ${PRODUCT_VERSION})

if(COMMAND find_host_program)
    find_host_program(PYTHON NAMES python python.exe REQUIRED)
else()
    find_program(PYTHON NAMES python python.exe REQUIRED)
endif()

string(REPLACE "_" ";" PRODUCT_FAMILY ${PRODUCT_ID})
list(GET PRODUCT_FAMILY 0 PRODUCT_FAMILY)
message(STATUS "PRODUCT_FAMILY: ${PRODUCT_FAMILY} (calculated from PRODUCT_ID)")

# generate VERSION_CODE from VERSION_NAME
MATH(EXPR VERSION_CODE "${BENCHMARK_VERSION_MAJOR} * 10000 + ${BENCHMARK_VERSION_MINOR} * 100 + ${BENCHMARK_VERSION_PATCH}")

# set PACKAGE_NAME, and PRODUCT_TYPE
if (OPT_COMMUNITY_BUILD)
    set(PRODUCT_TYPE "community")
    set(PACKAGE_NAME "net.kishonti.${PRODUCT_ID}")
else()
    set(PRODUCT_TYPE "corporate")
    # for corporate build append VERSION_CODE so different versions can be installed side by side.
    set(PACKAGE_NAME "net.kishonti.${PRODUCT_ID}.v${VERSION_CODE}.corporate")
endif()
# fixup PACKAGE_NAME to be valid reverse-DNS id
string(REPLACE "_" "." PACKAGE_NAME ${PACKAGE_NAME})

if ("$ENV{OBFUSCATE_PACKAGE_NAME}" STREQUAL "true")
    set(PACKAGE_NAME "cica.kutya.pianino")
endif()

# set up values for generator
set(MIN_SDK_VERSION "32")
set(REQUIRED_GLES_VERSION "0x00020000")

# generate files
set(GENERATED_JAVA_APP_DIR ${CMAKE_CURRENT_LIST_DIR}/src/main)
file(MAKE_DIRECTORY ${GENERATED_JAVA_APP_DIR})

# maintain backward compatibility here for specific benchmark versions that are already in the store
if ("${PRODUCT_ID}" MATCHES ".*gfxbench.*" AND STORE_VERSION)
    set(PACKAGE_NAME "net.kishonti.gfxbench")
endif()



# copy AndroidManifest.xml, and PRODUCT_TYPE specific files
configure_file(app-template/build.gradle.in ${CMAKE_CURRENT_LIST_DIR}/build.gradle @ONLY)
configure_file(app-template/AndroidManifest-${PRODUCT_TYPE}.xml.in ${GENERATED_JAVA_APP_DIR}/AndroidManifest.xml @ONLY)
if (OPT_COMMUNITY_BUILD)
    configure_file(app-template/Certificates-community.pem ${GENERATED_JAVA_APP_DIR}/assets/Certificates.pem COPYONLY)
endif()
file(COPY ${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw/resultdetail DESTINATION ${GENERATED_JAVA_APP_DIR}/assets/)



# copy PRODUCT_ID specific resources
if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/app-template/${PRODUCT_ID}/res)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/app-template/${PRODUCT_ID}/res ${GENERATED_JAVA_APP_DIR}/res)
endif()


# copy PRODUCT_VERSION specific resources
if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/app-template/${PRODUCT_ID}-${BENCHMARK_VERSION_MAJOR}.${BENCHMARK_VERSION_MINOR}/res)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/app-template/${PRODUCT_ID}-${BENCHMARK_VERSION_MAJOR}.${BENCHMARK_VERSION_MINOR}/res ${GENERATED_JAVA_APP_DIR}/res)
endif()


# copy testlist names
file(GLOB TESTLISTS "${TFW_PACKAGE_DIR}/${TESTLIST_NAME}*.json")
foreach(testlist ${TESTLISTS})
    # checking test resources (images)
    execute_process(COMMAND ${PYTHON} ${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw/support/benchmark_tests_util.py --image_ids --file ${testlist}
        OUTPUT_VARIABLE image_ids OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    if (NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${error}")
    endif()
    foreach(image_id ${image_ids})
        if(EXISTS ${TFW_PACKAGE_DIR}/images/${image_id}.png)
            file(COPY ${TFW_PACKAGE_DIR}/images/${image_id}.png DESTINATION ${GENERATED_JAVA_APP_DIR}/res/drawable/)
            message(STATUS "COPY ${TFW_PACKAGE_DIR}/images/${image_id}.png DESTINATION ${GENERATED_JAVA_APP_DIR}/res/drawable/")
        elseif(EXISTS ${TFW_PACKAGE_DIR}/images/${image_id}.xml)
            file(COPY ${TFW_PACKAGE_DIR}/images/${image_id}.xml DESTINATION ${GENERATED_JAVA_APP_DIR}/res/drawable/)
            message(STATUS "COPY ${TFW_PACKAGE_DIR}/images/${image_id}.xml DESTINATION ${GENERATED_JAVA_APP_DIR}/res/drawable/")
        else()
            message(FATAL_ERROR "Drawable resource not found: ${image_id}. PATH=${TFW_PACKAGE_DIR}/images/${image_id}.png")
        endif()
    endforeach()
endforeach()



# copy tfw-pkg content over the java project dir
foreach(dir libs)
    file(COPY ${TFW_PACKAGE_DIR}/${dir}/ DESTINATION ${GENERATED_JAVA_APP_DIR}/jniLibs)
endforeach()

file(REMOVE ${GENERATED_JAVA_APP_DIR}/jniLibs/gdb.setup)
file(REMOVE ${GENERATED_JAVA_APP_DIR}/jniLibs/gdbserver)
foreach(ARCH armeabi-v7a x86 arm64-v8a x86_64)
    file(REMOVE ${GENERATED_JAVA_APP_DIR}/jniLibs/${ARCH}/gdb.setup)
    file(REMOVE ${GENERATED_JAVA_APP_DIR}/jniLibs/${ARCH}/gdbserver)
endforeach()

# copy localization, benchmark_tests.json, config to assets
file(COPY
    ${TFW_PACKAGE_DIR}/localization
    ${TESTLISTS}
    ${TFW_PACKAGE_DIR}/config
    DESTINATION ${GENERATED_JAVA_APP_DIR}/assets/
)



# create the java app source project
set(TESTLIST_NAME "testlist")
if(${PRODUCT_FAMILY} STREQUAL "gfxbench")
    configure_file(app-template/res.values.colors_blue.xml ${GENERATED_JAVA_APP_DIR}/res/values/colors_blue.xml @ONLY)
elseif(${PRODUCT_FAMILY} STREQUAL "compubench")
    set(TESTLIST_NAME "benchmark_tests")
    configure_file(app-template/res.values.compubench.xml.in ${GENERATED_JAVA_APP_DIR}/res/values/${PRODUCT_ID}.xml @ONLY)
    configure_file(app-template/res.values.colors_green.xml ${GENERATED_JAVA_APP_DIR}/res/values/colors_green.xml @ONLY)
elseif(${PRODUCT_FAMILY} STREQUAL "adasbench")
	set(TESTLIST_NAME "benchmark_tests")
    configure_file(app-template/res.values.adasbench.xml.in ${GENERATED_JAVA_APP_DIR}/res/values/${PRODUCT_ID}.xml @ONLY)
    configure_file(app-template/res.values.colors_yellow.xml ${GENERATED_JAVA_APP_DIR}/res/values/colors_yellow.xml @ONLY)
else()
    message(FATAL_ERROR "No res.values.${PRODUCT_FAMILY}.xml.in found for PRODUCT_ID: ${PRODUCT_ID}")
endif()


# copy data dir if corporate
if (NOT COMMUNITY_BUILD)
    file(COPY ${TFW_PACKAGE_DIR}/data DESTINATION ${GENERATED_JAVA_APP_DIR}/assets/)
endif()





if ("$ENV{ARCHIVE_ROOT}" STREQUAL "")
    set(APK_INSTALL_DIR ${TFW_PACKAGE_DIR})
else()
    set(APK_INSTALL_DIR $ENV{ARCHIVE_ROOT})
endif()

set(KEYS_DIR ${CMAKE_CURRENT_LIST_DIR}/../frameworks/keys)
if (STORE_VERSION)
    message(STATUS "STORE_VERSION: signing app with release key")
    configure_file(${KEYS_DIR}/kishonti.keystore ${GENERATED_JAVA_APP_DIR}/ COPYONLY)
    set(STORE_NAME "GooglePlay")
else()
    message(STATUS "NOT STORE_VERSION: signing app with debug key")
    set(STORE_NAME "localBuild")
endif()

configure_file(app-template/res.values.${PRODUCT_FAMILY}.xml.in ${GENERATED_JAVA_APP_DIR}/res/values/${PRODUCT_ID}.xml @ONLY)

# Set up gradle
set(GRADLE_EXECUTABLE ${CMAKE_CURRENT_LIST_DIR}/gradlew)
execute_process(COMMAND chmod +x ${GRADLE_EXECUTABLE})

if (OPT_COMMUNITY_BUILD)
    # Create bundle
    set(aab_path ${CMAKE_CURRENT_LIST_DIR}/build/outputs/bundle/${PRODUCT_ID}-${PRODUCT_VERSION}+${PRODUCT_TYPE}${archs}.aab)
    add_custom_command(OUTPUT command_aab
        COMMAND ${GRADLE_EXECUTABLE} --warning-mode all -p ${CMAKE_CURRENT_LIST_DIR} bundleRelease
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_LIST_DIR}/build/outputs/bundle/release/GFXBench-release.aab ${aab_path}
    )
    message(STATUS "Creating target: aab")
    install(FILES ${aab_path} DESTINATION ${APK_INSTALL_DIR})
    set(AAB command_aab)
endif()

# Create the apk
set(apk_path ${CMAKE_CURRENT_LIST_DIR}/build/outputs/apk/${PRODUCT_ID}-${PRODUCT_VERSION}+${PRODUCT_TYPE}${archs}.apk)
add_custom_command(OUTPUT command_apk
    COMMAND ${GRADLE_EXECUTABLE} --warning-mode all -p ${CMAKE_CURRENT_LIST_DIR} assembleRelease
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_LIST_DIR}/build/outputs/apk/release/GFXBench-release.apk ${apk_path}
)
message(STATUS "Creating target: apk")
install(FILES ${apk_path} DESTINATION ${APK_INSTALL_DIR})
add_custom_target(apk ALL DEPENDS command_apk ${AAB})
