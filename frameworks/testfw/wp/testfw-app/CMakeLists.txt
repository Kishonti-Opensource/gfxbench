cmake_minimum_required(VERSION 3.1.1)

project(StoreApp)

include(mklink)
include(VersionParse)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

version_parse(BENCHMARK ${PRODUCT_VERSION})

#------------- linking ----------------------------------
mklink("${WORKSPACE}/tfw-pkg" 						"${CMAKE_BINARY_DIR}/resources")
mklink("${CMAKE_CURRENT_LIST_DIR}/cmake" 			"${CMAKE_BINARY_DIR}/cmake")
mklink("${CMAKE_CURRENT_LIST_DIR}/packages" 		"${CMAKE_BINARY_DIR}/packages")
mklink("${CMAKE_CURRENT_LIST_DIR}/src" 				"${CMAKE_BINARY_DIR}/src")
mklink("${CMAKE_CURRENT_LIST_DIR}/WindowsPhoneApp" 	"${CMAKE_BINARY_DIR}/WindowsPhoneApp")

#------------ generating manifest file ------------------
set(MANIFEST_DIR "WindowsPhoneApp")
set(APP_MANIFEST_NAME Package.appxmanifest)
set(EXE_NAME GFXBenchAutoTest)
set(SHORT_NAME "testfw_app")
set(PACKAGE_VERSION "${BENCHMARK_VERSION_MAJOR}.${BENCHMARK_VERSION_MINOR}.${BENCHMARK_VERSION_PATCH}.0")
set(IDENTITY_NAME "testfw.app")
set(PUBLISHER_NAME "CN=Kishonti Kft.")
set(PACKAGE_GUID "dcfce070-e7d3-4094-8752-a8aa91fe18de")

configure_file(
	"${CMAKE_BINARY_DIR}/cmake/${APP_MANIFEST_NAME}.in"
	"${CMAKE_CURRENT_BINARY_DIR}/${MANIFEST_DIR}/${APP_MANIFEST_NAME}"
@ONLY)

#-------------------------------------------------------
include_external_msproject(1_WindowsPhoneInterop ${CMAKE_BINARY_DIR}/WindowsPhoneApp/WindowsPhoneInterop.vcxproj)
include_external_msproject(2_WindowsPhoneApp ${CMAKE_BINARY_DIR}/WindowsPhoneApp/WindowsPhoneApp.csproj)
add_library(WPInteropConnector SHARED dummy.cpp)
add_dependencies(WPInteropConnector testfw_cs gfxbench30_dx)

#--------------- install -------------------------------
install(DIRECTORY WindowsPhoneApp/AppPackages DESTINATION .)
install(DIRECTORY WindowsPhoneApp/AppPackages DESTINATION ${TFW_PACKAGE_DIR})

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_FILE_NAME "${PRODUCT_ID}-$ENV{NG_TARGET_PLATFORM}-wp-${PRODUCT_VERSION}+corporate")
set(CPACK_OUTPUT_FILE_PREFIX "${TFW_PACKAGE_DIR}")
include(CPack)
