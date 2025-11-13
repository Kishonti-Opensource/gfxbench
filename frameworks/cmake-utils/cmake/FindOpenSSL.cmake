include(TryFindPackageCfg)
try_find_package_cfg()

if(OPENSSL_FOUND)
	return()
endif()

include(IncludeStandardFindModule)
include_standard_find_module()
