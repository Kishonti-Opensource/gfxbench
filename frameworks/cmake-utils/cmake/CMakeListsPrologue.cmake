# General settings for main CMakeLists
# Include this at the beginning of every main CMakeLists.txt
# to set up compiler flags, handle general options, etc

if (NOT CMakeListsPrologue_included)
	set(CMakeListsPrologue_included 1)
	set(CMakeListsPrologue_dir ${CMAKE_CURRENT_LIST_DIR})

	include(${CMAKE_CURRENT_LIST_DIR}/printvar.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/OptionFromEnv.cmake)

	printvar(CMAKE_PREFIX_PATH)
	printenvvar(CMAKE_PREFIX_PATH)
	printvar(CMAKE_FIND_ROOT_PATH)
	printvar(CMAKE_INSTALL_PREFIX)
	printvar(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM)
	printvar(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE)
	printvar(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY)
	printvar(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE)
	printvar(CMAKE_MODULE_PATH)

	#find proto libraries in cmake prefix path
	foreach(i ${CMAKE_PREFIX_PATH} $ENV{CMAKE_PREFIX_PATH})
		find_path(PROTOBUF_SRC_ROOT_FOLDER find_path_target "${i}/lib/protobuf")
	endforeach()
	if ( PROTOBUF_SRC_ROOT_FOLDER )
		message(STATUS "PROTOBUF_SRC_ROOT_FOLDER set to ${PROTOBUF_SRC_ROOT_FOLDER}")
	endif()

	set(CMAKE_FIND_FRAMEWORK "LAST")

	if(CMAKE_CXX_COMPILER)
		#optionally set _DEBUG
		string(REGEX MATCH [-/]D_DEBUG rxmatch "${CMAKE_CXX_FLAGS_DEBUG}")
		if(NOT rxmatch)
			set_property(CACHE CMAKE_CXX_FLAGS_DEBUG PROPERTY VALUE "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
		endif()
	endif()

	string(REGEX MATCH [-/]D_DEBUG rxmatch "${CMAKE_C_FLAGS_DEBUG}")
	if(NOT rxmatch)
		set_property(CACHE CMAKE_C_FLAGS_DEBUG PROPERTY VALUE "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
	endif()

	if(MSVC)
		add_definitions(
			-D_SCL_SECURE_NO_WARNINGS
			-D_CRT_SECURE_NO_WARNINGS
			-D_CRT_SECURE_NO_WARNINGS
			)
		# Under Windows CE NOMINMAX is already defined
		if (NOT WINCE)
			add_definitions(
				-DNOMINMAX
			)
		else()
			add_definitions(
				-DUNICODE -D_UNICODE
			)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FI altcecrt.h")
			set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /DDEBUG")
			if(CMAKE_CXX_COMPILER)
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FI altcecrt.h")
				set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG")
			endif()
		endif()
		if(CMAKE_CXX_COMPILER)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4146 /we4800 /we4172 /we4715 /we4150 /we4290")
			if (NOT WINCE)
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
			endif()
		endif()

		if (MSVC_VERSION LESS 1600)
			find_package(msinttypes REQUIRED)
			include_directories(${MSINTTYPES_INCLUDE_DIRS})
			add_definitions(${MSINTTYPES_DEFINITIONS})
		endif()

	endif(MSVC)

	if (WIN32 AND CMAKE_COMPILER_IS_GNUCC)
		if (NOT MINGW)
			message(WARNING "Gee, MINGW was not set by cmake")
			set(MINGW 1)
		endif()
	endif()

	include(${CMAKE_CURRENT_LIST_DIR}/private/InitNGLog.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/apply_find_vars.cmake)
	include(${CMAKE_CURRENT_LIST_DIR}/BuildTag.cmake)
endif()

