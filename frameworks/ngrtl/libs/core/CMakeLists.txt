set(srcs
	src/require.cpp
	src/format.cpp
	
	src/stream/bytestream.cpp
	src/stream/filestreambuf.cpp
	src/stream/memstreambuf.cpp
	src/stream/textstream.cpp

	src/log/log.cpp
	
	src/bytevec.cpp

	src/json/deserialize.cpp
	src/json/json.cpp
	src/json/JSON_parser.c
	src/json/serialize.cpp


	src/mutex.cpp #need for log.cpp
	src/timer.cpp

	src/base64/cdecode.c
	src/base64/cencode.c
	src/base64/decode.cpp
	src/base64/encode.cpp
	src/stringutil.cpp
	src/cstring.cpp
	src/substring.cpp
	src/macros.cpp
	src/object.cpp
	src/var.cpp

	src/json_var/json_parse.cpp
	src/json_var/json_stringify.cpp
	src/json_var/js_json.cpp

	src/result.cpp
	src/frequency_counter.cpp
	src/hr_clock.cpp
)

file(GLOB_RECURSE hdrs1 src/*.h)
file(GLOB_RECURSE hdrs2 ${CMAKE_CURRENT_SOURCE_DIR}/../../include/core/*.h)

if(WINCE)
	find_package(wcecompat REQUIRED)
	include_directories(${WCECOMPAT_INCLUDE_DIRS})
endif()

add_library(ngrtl_core ${srcs} ${hdrs1} ${hdrs2})

if(ANDROID)
	# Logger depends on log library on Android
	set(NGRTL_CORE_DEPENDENT_LIBRARIES -llog PARENT_SCOPE)
	target_link_libraries(ngrtl_core log)
endif()

if(UNIX AND NOT APPLE AND NOT IOS AND NOT ANDROID AND NOT QNX)
	# Depends on pthreads library on Linux
	set(NGRTL_CORE_DEPENDENT_LIBRARIES "-pthread -lrt" PARENT_SCOPE)
	target_link_libraries(ngrtl_core -pthread rt)
endif()

install_ngrtl_target(core)

