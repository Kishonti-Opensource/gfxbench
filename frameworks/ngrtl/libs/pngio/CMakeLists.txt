if(NGRTL_ENABLE_PNGIO)
	set(R REQUIRED)
else()
	set(R QUIET)
endif()

find_package(PNG ${R})

if (PNG_FOUND)
	apply_find_vars(PNG)
	set(SRCS pngio.cpp)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/pngio)
	
	add_library(ngrtl_pngio STATIC ${SRCS})

	target_link_libraries(ngrtl_pngio ngrtl_core)
	target_link_libraries(ngrtl_pngio ${APPLY_FIND_LIBRARIES})
	set(NGRTL_PNGIO_DEPENDENT_FIND_PACKAGES PNG PARENT_SCOPE)

	install_ngrtl_target(pngio)
else()
	message("PNG not found, skipping ngrtl/pngio")
endif()
