project(ngrtl)
cmake_minimum_required(VERSION 3.5)

include(CMakePrintHelpers)

# call find_package cmake-utils only if CMAKE_MODULE_PATH doesn't contain cmake-utils/cmake
if(CMAKE_MODULE_PATH)
	cmake_print_variables(CMAKE_MODULE_PATH)
	find_file(NGRTL_CMAKELISTSPROLOGUE_PATH CMakeListsPrologue.cmake PATHS ${CMAKE_MODULE_PATH} NO_DEFAULT_PATH)
	mark_as_advanced(NGRTL_CMAKELISTSPROLOGUE_PATH)
	if(NOT NGRTL_CMAKELISTSPROLOGUE_PATH)
		message(STATUS "CMakeListsPrologue.cmake not found on CMAKE_MODULE_PATH. Searching again.")
		foreach(p ${CMAKE_MODULE_PATH})
			set(pp "${p}/CMakeListsPrologue.cmake")
			if(EXISTS "${pp}")
				set(NGRTL_CMAKELISTSPROLOGUE_PATH "${pp}")
				message(STATUS "Yet it has been found at ${pp}")
				break()
			else()
				message(STATUS "Not found at ${pp}")
			endif()
		endforeach()
	endif()
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /W3 /WX /wd4275 /wd4244 /wd4838 /wd4267")
endif()

if(NOT NGRTL_CMAKELISTSPROLOGUE_PATH)
	find_package(cmake-utils REQUIRED append_module_path)
endif()

include(CMakePrintHelpers)

set(CMAKE_MACOSX_RPATH 1)

include(${CMAKE_CURRENT_LIST_DIR}/decide_which_modules.cmake)

if(NGRTL_MODULES)
  cmake_print_variables(NGRTL_MODULES)
endif()

message(STATUS "Building these modules:")
foreach(m ${all_modules})
	string(TOUPPER "${m}" M)
	if(NGRTL_ENABLE_${M})
		message(STATUS "${m}")
	endif()
endforeach()

#HACK, no exceptions allowed
#TODO fix this quick workaround for wince (problem: variadic arguments for macros apparently not supported)
#this comes before including cmakelistsprologue
set(OPT_NGLOG_ENABLE NGLOG_SEVERITY_ANY CACHE STRING "OPT_NGLOG_ENABLE must be NGLOG_SEVERITY_ANY when building ngrtl" FORCE)

include(CMakeListsPrologue)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/core)

if(MSVC AND NOT WINCE)
	add_definitions(-DNOMINMAX)
endif()

if(NOT BUILD_SHARED_LIBS)
	add_definitions(-DNGRTL_STATIC)
endif()

if (MSVC90 OR MSVC80)
	find_package(msinttypes REQUIRED)
	apply_find_vars(MSINTTYPES)
endif()

function(install_ngrtl_target sl)
	set(t ngrtl_${sl})
	set_target_properties(${t} PROPERTIES DEBUG_POSTFIX _d)

	install(TARGETS ${t} DESTINATION lib)

	# the PUBLIC_HEADERS property doesn't keep dir tree, need to install with as DIRECTORY
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${sl} DESTINATION include/ngrtl)
endfunction()

unset(NGRTL_DEPENDENCY_LISTING)
foreach(sl ${all_modules})
  string(TOUPPER "${sl}" M)
  if(NGRTL_ENABLE_${M})
		add_subdirectory(libs/${sl})
		string(TOUPPER ${sl} SL)
		set(v1 NGRTL_${SL}_DEPENDENT_FIND_PACKAGES)
		set(v1b NGRTL_${SL}_DEPENDENT_FIND_PACKAGES_WITH_INCLUDE_DIRS)
		set(v2 NGRTL_${SL}_DEPENDENT_LIBRARIES)
		if (${v1})
			set(NGRTL_DEPENDENCY_LISTING "${NGRTL_DEPENDENCY_LISTING}\nset(${v1} \"${${v1}}\")")
		endif()
		if (${v1b})
			set(NGRTL_DEPENDENCY_LISTING "${NGRTL_DEPENDENCY_LISTING}\nset(${v1b} \"${${v1b}}\")")
		endif()
		if (${v2})
			set(NGRTL_DEPENDENCY_LISTING "${NGRTL_DEPENDENCY_LISTING}\nset(${v2} \"${${v2}}\")")
		endif()
	endif()
endforeach()

configure_file(ngrtlConfig.cmake.in ngrtlConfig.cmake.out @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ngrtlConfig.cmake.out DESTINATION share/ngrtl/cmake RENAME ngrtlConfig.cmake)
