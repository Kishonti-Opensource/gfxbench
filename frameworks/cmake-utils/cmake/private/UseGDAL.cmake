FUNCTION(run_use_module this)

	if(WIN32)
		set(lrp $ENV{NG_THIRDPARTY_ROOT})

		if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 4)
			set(p "${lrp}/windows/lib/GDAL-1.9.0")
		elseif(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
			set(p "${lrp}/windows64/libraries/GDAL")
		else()
			message(FATAL_ERROR "Invalid sizeof(void*): ${CMAKE_CXX_SIZEOF_DATA_PTR}")
		endif()
		
		set(ids "${p}/include")
		set(lib "${p}/lib/gdal_i.lib")
		if(EXISTS ${ids} AND EXISTS ${lib})
			object_set(${this} include_dirs "${ids}")
			object_set(${this} libraries "general;${lib}")
			object_set(${this} found 1)
			object_set(${this} definitions "-DCPL_DLL=")
		endif()
	else()
	
		use_to_find_caller(${this} GDAL)
		
		if(GDAL_FOUND)
			object_set(${this} found 1)
			object_set(${this} include_dirs ${GDAL_INCLUDE_DIRS})
			object_set(${this} libraries ${GDAL_LIBRARIES})
		endif()
		
	endif()

ENDFUNCTION()
