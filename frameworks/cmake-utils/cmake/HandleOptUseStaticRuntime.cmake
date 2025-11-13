# This module handles the OPT_STATIC_RUNTIME cache variable
# If it's on it switches on static runtime building
# It works only for MSVC. Please complete for other compilers as needed
#
# The state of OPT_STATIC_RUNTIME have the following effects (at least)
#
# - Boost_USE_STATIC_RUNTIME is set accordingly (in this module)
# - BuildTag generates build tags with 's' runtime-lib tag if it's ON

if(NOT DEFINED OPT_USE_STATIC_RUNTIME)
	option(OPT_USE_STATIC_RUNTIME "Link with static runtime libs" OFF)
	set(OPT_USE_STATIC_RUNTIME_PREVIOUS ON CACHE INTERNAL "Previous state of OPT_USE_STATIC_RUNTIME" FORCE)
endif()

if (OPT_USE_STATIC_RUNTIME AND NOT MSVC)
	message(FATAL_ERROR "the OPT_USE_STATIC_RUNTIME flag is handled  only for MSVC compiler. If you need it for other compiler please add it to the HandleOptUseStaticRuntime.cmake file")
endif()

# if setting changed, reset FindBoost to trigger new find
if (NOT (OPT_USE_STATIC_RUNTIME EQUAL OPT_USE_STATIC_RUNTIME_PREVIOUS) )
	set(OPT_USE_STATIC_RUNTIME_PREVIOUS ${OPT_USE_STATIC_RUNTIME})
	include(${CMAKE_CURRENT_LISTS_DIR}/ResetFindBoost.cmake)
endif()

set(Boost_USE_STATIC_RUNTIME ${OPT_USE_STATIC_RUNTIME})

if (MSVC)
	foreach(i IN LISTS CMAKE_CONFIGURATION_TYPES)
		string(TOUPPER ${i} upper_i)
		set(flagsvarname CMAKE_CXX_FLAGS_${upper_i})
		get_property(flags CACHE ${flagsvarname} PROPERTY VALUE)
		if(OPT_USE_STATIC_RUNTIME)
			string(REPLACE "/MDd" "/MTd" flags2 "${flags}")
			string(REPLACE "/MD" "/MT" flags3 "${flags2}")
		else()
			string(REPLACE "/MTd" "/MDd" flags2 "${flags}")
			string(REPLACE "/MT" "/MD" flags3 "${flags2}")
		endif()
		set_property(CACHE ${flagsvarname} PROPERTY VALUE ${flags3})
	endforeach()
endif()