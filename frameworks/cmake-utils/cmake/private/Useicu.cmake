FUNCTION(run_use_module this)

	if(WIN32)
	
		set(lrp $ENV{NG_THIRDPARTY_ROOT})
		
		if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 4)
			set(root "${lrp}/windows/libraries/icu")
			set(p "")
		elseif(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
			set(root "${lrp}/windows64/libraries/icu")
			set(p 64)
		else()
			message(FATAL_ERROR "Invalid sizeof(void*): ${CMAKE_CXX_SIZEOF_DATA_PTR}")
		endif()

		set(libroot "${root}/lib${p}")

		set(comps icuuc)
		object_get0(${this} components components)

		foreach(i IN LISTS components)
			if (i STREQUAL i18n)
				list(APPEND comps icuin)
			elseif ( i STREQUAL io )
				list(APPEND comps icuio)
			elseif ( i STREQUAL layoutex )
				list(APPEND comps icule)
			elseif ( i STREQUAL layout )
				list(APPEND comps iculx)
			endif()
		endforeach()

		set(libs general "${libroot}/icudt.lib")
		foreach(i IN LISTS comps)
			list(APPEND libs general "${libroot}/${i}.lib")
#			list(APPEND libs optimized "${libroot}/${i}.lib")
#			list(APPEND libs debug "${libroot}/${i}d.lib")
		endforeach()
		
		object_set(${this} include_dirs "${root}/include")
		object_set(${this} libraries "${libs}")
		object_set(${this} found 1)
	else()
		use_to_find_caller(${this} ICU)
		
		if(ICU_FOUND)
			object_set(${this} found 1)
			object_set(${this} include_dirs ${ICU_INCLUDE_DIRS})
			object_set(${this} libraries ${ICU_LIBRARIES})
		endif()
	endif()

ENDFUNCTION()
