cmake_minimum_required(VERSION 3.5)

project(agilitysdkpreview)

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for dlls")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")


include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)

if($ENV{NG_TARGET_PLATFORM} MATCHES "vs20[0-9][0-9]-arm64")
	set(AGILITY_CORE_DLL
		${CMAKE_CURRENT_LIST_DIR}/bin/ARM64/D3D12Core.dll
	)
	set(AGILITY_DEBUG_LAYER
		${CMAKE_CURRENT_LIST_DIR}/bin/ARM64/d3d12SDKLayers.dll
	)
else()
	set(AGILITY_CORE_DLL
		${CMAKE_CURRENT_LIST_DIR}/bin/x64/D3D12Core.dll
	)
	set(AGILITY_DEBUG_LAYER
		${CMAKE_CURRENT_LIST_DIR}/bin/x64/d3d12SDKLayers.dll
	)
endif()

install(DIRECTORY include/ DESTINATION include)
install(FILES ${AGILITY_CORE_DLL} DESTINATION "${INSTALL_BIN_DIR}/D3D12")
install(FILES ${AGILITY_DEBUG_LAYER} DESTINATION "${INSTALL_BIN_DIR}/D3D12")
