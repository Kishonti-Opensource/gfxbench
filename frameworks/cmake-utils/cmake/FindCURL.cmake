# - find curl
# sets:
# CURL_INCLUDE_DIRS
# CURL_LIBRARIES
# CURL_FOUND
# CURL_DEFINITIONS
include(TryFindPackageCfg)
try_find_package_cfg()

if(CURL_FOUND)
	return()
endif()

if(MSVC OR MINGW)
	include(FindAnyStandardLib)
	if (WIN32 AND CMAKE_COMPILER_IS_GNUCC)
		set(CURL_BASEDIR $ENV{NG_THIRDPARTY_ROOT}/mingw)
		find_path(CURL_INCLUDE_DIRS curl/curl.h HINTS ${CURL_BASEDIR}/include)
		find_library(CURL_LIBRARIES libcurl.lib HINTS ${CURL_BASEDIR}/lib)
	elseif(WINCE)
		set(WINCE_PATH $ENV{NG_THIRDPARTY_ROOT}/wince)
		#message("WINCE_PATH=$ENV{NG_THIRDPART_ROOT}/wince")
		find_path(CURL_INCLUDE_DIRS curl/curl.h PATHS ${WINCE_PATH}/include CMAKE_FIND_ROOT_PATH_BOTH)
		#message(FATAL_ERROR CURL_INCLUDE_DIRS=${CURL_INCLUDE_DIRS})
		find_library (CURL_LIBRARIES libcurl_imp.lib PATHS ${WINCE_PATH}/lib CMAKE_FIND_ROOT_PATH_BOTH)
		#message(FATAL_ERROR CURL_LIBRARIES=${CURL_LIBRARIES})
		set(CURL_FOUND YES)
	else()
		find_any_standard_lib(CURL $ENV{NG_THIRDPARTY_ROOT} libcurl)
		if (CURL_FOUND_STATIC)
			set(CURL_DEFINITIONS -DCURL_STATICLIB)
			list(APPEND CURL_LIBRARIES wldap32.lib)
		endif()
	endif()

	find_any_standard_lib_coda(CURL)
else()
	include(IncludeStandardFindModule)
	include_standard_find_module()
endif()