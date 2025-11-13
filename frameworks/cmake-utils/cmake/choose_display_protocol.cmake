cmake_policy(SET CMP0054 NEW)

if( "${DISPLAY_PROTOCOL}" STREQUAL "UNSET" )

elseif( "${DISPLAY_PROTOCOL}" STREQUAL "NONE" )
	add_definitions(-DDISPLAY_PROTOCOL_NONE=1)

elseif( "${DISPLAY_PROTOCOL}" STREQUAL "WIN32" )
	add_definitions(-DDISPLAY_PROTOCOL_WIN32=1)

elseif( "${DISPLAY_PROTOCOL}" STREQUAL "ANDROID" )
	add_definitions(-DDISPLAY_PROTOCOL_ANDROID=1)

elseif( "${DISPLAY_PROTOCOL}" STREQUAL "SCREEN" )
	list(APPEND sources
		${dir}/graphics/qnxgraphicswindow.h
		${dir}/graphics/qnxgraphicswindow.cpp
	)
	add_definitions(-DDISPLAY_PROTOCOL_SCREEN=1)

elseif( "${DISPLAY_PROTOCOL}" STREQUAL "XCB" )
	find_package(XCB REQUIRED)
	list(APPEND sources
		${dir}/graphics/xcbgraphicswindow.h
		${dir}/graphics/xcbgraphicswindow.cpp        
		)
	add_definitions(-DDISPLAY_PROTOCOL_XCB=1)
	list(APPEND platform_libraries ${XCB_LIBRARIES} )

elseif( "${DISPLAY_PROTOCOL}" STREQUAL "WAYLAND" )
	find_package(Wayland REQUIRED)
	list(APPEND sources
		${dir}/graphics/wlgraphicswindow.h
		${dir}/graphics/wlgraphicswindow.cpp
	)
	add_definitions(-DDISPLAY_PROTOCOL_WAYLAND=1)
	list(APPEND platform_libraries ${WAYLAND_LIBRARIES})
endif()
