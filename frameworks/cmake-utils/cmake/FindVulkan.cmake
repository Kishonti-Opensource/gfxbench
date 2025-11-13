# To install this, place in [Project root]/cmake/Modules/ or <CMAKE_ROOT>/share/cmake-x.y/Modules/
# - Try to find Vulkan SDK
# If successful, will define:
#	VULKAN_FOUND
#	VULKAN_INCLUDE_DIRS
#	VULKAN_LIBRARIES
#	VULKAN_DEFINITIONS

find_package(PkgConfig)

set(VULKAN_DEFINITIONS) # nothing

find_path(VULKAN_INCLUDE_DIR
	vulkan/vulkan.h
	$ENV{VULKAN_SDK}/Include
	)

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
	find_library(VULKAN_LIBRARY vulkan-1 names vulkan
		PATHS $ENV{VULKAN_SDK}
		HINTS $ENV{VULKAN_SDK}
		PATH_SUFFIXES bin lib)
else()
	find_library(VULKAN_LIBRARY vulkan-1 names vulkan
		PATHS $ENV{VULKAN_SDK}
		HINTS $ENV{VULKAN_SDK}
		PATH_SUFFIXES bin32 lib32
)
endif()

set(VULKAN_LIBRARIES ${VULKAN_LIBRARY})
set(VULKAN_INCLUDE_DIRS ${VULKAN_INCLUDE_DIR})

find_package_handle_standard_args(Vulkan DEFAULT_MSG 
	VULKAN_LIBRARY VULKAN_INCLUDE_DIR)

mark_as_advanced(VULKAN_INCLUDE_DIR VULKAN_LIBRARY)
