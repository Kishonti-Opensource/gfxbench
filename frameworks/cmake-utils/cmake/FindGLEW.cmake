include(TryFindPackageCfg)
try_find_package_cfg()

if(GLEW_FOUND)
	return()
endif()

include(IncludeStandardFindModule)
include_standard_find_module()
