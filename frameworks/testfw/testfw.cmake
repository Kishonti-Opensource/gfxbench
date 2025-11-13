set(TESTFW_MODULES2 "" CACHE STRING "hack&workaround" )

function(register_testfw_module2 module test_names)
    set(TESTFW_MODULES2 ${module} CACHE STRING "" FORCE)
endfunction()

if(testfw_cmake_include_guard)
    return()
endif()

set(TESTFW_MSVC_INCLUDE_SYMBOLS "" CACHE STRING "Test factory symbols" FORCE)
set(TESTFW_LIBRARIES "" CACHE STRING "Test linker command line" FORCE)
set(TESTFW_MODULES "" CACHE STRING "Test Modules" FORCE)
set(TESTFW_TESTS "" CACHE STRING "Test ids" FORCE)
message(STATUS "testfw.cmake running: ${CMAKE_CURRENT_SOURCE_DIR}")

function(register_testfw_module module test_names)
	message("Registering testfw module: ${module}")
	message("  test: ${test_names}")
	get_target_property(TYPE "${module}" "TYPE")
	set(TESTFW_MODULES ${TESTFW_MODULES} ${module} ${TESTFW_MODULES2} CACHE STRING "Test Modules" FORCE)
	set(TESTFW_TESTS ${TESTFW_TESTS} ${test_names} CACHE STRING "Test ids" FORCE)
	set_target_properties(${module} PROPERTIES TESTFW_TESTS "${test_names}")
	if ("${TYPE}" MATCHES "STATIC_LIBRARY")
		if(APPLE)

if(TESTFW_MODULES2)
			set(TESTFW_LIBRARIES -Wl,-force_load ${TESTFW_MODULES2})
endif()
			set(TESTFW_LIBRARIES ${TESTFW_LIBRARIES} -Wl,-force_load ${module} CACHE STRING "Test linker command line" FORCE)

		elseif(MSVC)
			foreach(test ${test_names})
				if(CMAKE_SIZEOF_VOID_P EQUAL 8)
					set(TESTFW_MSVC_INCLUDE_SYMBOLS "${TESTFW_MSVC_INCLUDE_SYMBOLS} /INCLUDE:create_test_${test}" CACHE STRING "Factory method names" FORCE)
				else()
					set(TESTFW_MSVC_INCLUDE_SYMBOLS "${TESTFW_MSVC_INCLUDE_SYMBOLS} /INCLUDE:_create_test_${test}" CACHE STRING "Factory method names" FORCE)
				endif()
			endforeach()
			set(TESTFW_LIBRARIES ${TESTFW_LIBRARIES} ${module} CACHE STRING "Test Modules" FORCE)
		elseif(CMAKE_COMPILER_IS_GNUCC)
			set(TESTFW_LIBRARIES ${TESTFW_LIBRARIES} -Wl,-whole-archive ${module} -Wl,-no-whole-archive CACHE STRING "Test linker command line" FORCE)
		else()
			message(WARNING "Please make sure your linker will keep symbols from static library: ${module}")
			set(TESTFW_LIBRARIES ${TESTFW_LIBRARIES} ${module} CACHE STRING "Test linker command line" FORCE)
		endif()
	elseif("${TYPE}" MATCHES "SHARED_LIBRARY" AND TFW_PACKAGE_DIR)
		add_custom_target(copy_plugins_${module} ALL DEPENDS ${module})

		if(ANDROID)
			add_custom_command(TARGET copy_plugins_${module}
				COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${module}> ${TFW_PACKAGE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/$<TARGET_FILE_NAME:${module}>)
		else()
			add_custom_command(TARGET copy_plugins_${module}
				COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${module}> ${TFW_PACKAGE_DIR}/plugins/$<TARGET_FILE_NAME:${module}>)
		endif()

	endif()
endfunction()


macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child} AND EXISTS ${curdir}/${child}/CMakeLists.txt)
        set(dirlist ${dirlist} ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()


set(testfw_cmake_include_guard 1)
