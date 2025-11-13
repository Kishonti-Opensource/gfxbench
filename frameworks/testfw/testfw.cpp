/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testfw.h"
#ifdef WIN32
#include <windows.h>
#include "winapifamilynull.h"
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <Psapi.h>
#endif
#include <vector>
#else
#include <dlfcn.h>
#endif
#include <string>
#include <iostream>
#include "ng/log.h"

#if WIN32
extern "C" { __declspec(dllexport) extern unsigned int D3D12SDKVersion = 610; }
extern "C" { __declspec(dllexport) extern char* D3D12SDKPath = u8".\\D3D12\\"; }
#endif

namespace tfw {

void* TestFactory::gfx5_handle = nullptr;
void* TestFactory::gfx4_handle = nullptr;

TestFactory::TestFactory()
	: factory_method_(0)
{
}

#if defined(__QNX__)
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_5();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_4();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_4_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_alu2();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_alu2_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_alu();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_alu_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_blending();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_blending_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_driver2();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_driver2_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_driver();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_driver_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_egypt();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_egypt_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_fill2();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_fill2_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_fill();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_fill_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_manhattan31_battery();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_manhattan31();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_manhattan31_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_manhattan();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_manhattan_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_tess();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_tess_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_precision();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_trex_battery();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_trex();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_trex_off();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_trex_qmatch_highp();
extern "C" APICALL tfw::TestFactory::Holder create_test_gl_trex_qmatch();
#endif /* __QNX__ */

TestFactory TestFactory::test_factory(const char *name)
{
	std::string factory("create_test_");
	factory += name;
	void *fn = 0;
#if __APPLE__
	#if __arm__ || __arm64__
		fn = dlsym(RTLD_MAIN_ONLY, factory.c_str());
	#else
		fn = dlsym(RTLD_DEFAULT, factory.c_str());
	#endif
#elif linux || __ANDROID__ || __QNX__ || defined NACL || defined __linux__ || defined __linux
	fn = dlsym(RTLD_DEFAULT, factory.c_str());
#if defined(__QNX__)
	/* We have broken dlsym for non shared objects */
	if (factory.compare("create_test_gl_5") == 0) {
		fn = (void*)create_test_gl_5;
	}
	if (factory.compare("create_test_gl_4") == 0) {
		fn = (void*)create_test_gl_4;
	}
	if (factory.compare("create_test_gl_4_off") == 0) {
		fn = (void*)create_test_gl_4_off;
	}
	if (factory.compare("create_test_gl_alu2") == 0) {
		fn = (void*)create_test_gl_alu2;
	}
	if (factory.compare("create_test_gl_alu2_off") == 0) {
		fn = (void*)create_test_gl_alu2_off;
	}
	if (factory.compare("create_test_gl_alu") == 0) {
		fn = (void*)create_test_gl_alu;
	}
	if (factory.compare("create_test_gl_alu_off") == 0) {
		fn = (void*)create_test_gl_alu_off;
	}
	if (factory.compare("create_test_gl_blending") == 0) {
		fn = (void*)create_test_gl_blending;
	}
	if (factory.compare("create_test_gl_blending_off") == 0) {
		fn = (void*)create_test_gl_blending_off;
	}
	if (factory.compare("create_test_gl_driver2") == 0) {
		fn = (void*)create_test_gl_driver2;
	}
	if (factory.compare("create_test_gl_driver2_off") == 0) {
		fn = (void*)create_test_gl_driver2_off;
	}
	if (factory.compare("create_test_gl_driver") == 0) {
		fn = (void*)create_test_gl_driver;
	}
	if (factory.compare("create_test_gl_driver_off") == 0) {
		fn = (void*)create_test_gl_driver_off;
	}
	if (factory.compare("create_test_gl_egypt") == 0) {
		fn = (void*)create_test_gl_egypt;
	}
	if (factory.compare("create_test_gl_egypt_off") == 0) {
		fn = (void*)create_test_gl_egypt_off;
	}
	if (factory.compare("create_test_gl_fill2") == 0) {
		fn = (void*)create_test_gl_fill2;
	}
	if (factory.compare("create_test_gl_fill2_off") == 0) {
		fn = (void*)create_test_gl_fill2_off;
	}
	if (factory.compare("create_test_gl_fill") == 0) {
		fn = (void*)create_test_gl_fill;
	}
	if (factory.compare("create_test_gl_fill_off") == 0) {
		fn = (void*)create_test_gl_fill_off;
	}
	if (factory.compare("create_test_gl_manhattan31_battery") == 0) {
		fn = (void*)create_test_gl_manhattan31_battery;
	}
	if (factory.compare("create_test_gl_manhattan31") == 0) {
		fn = (void*)create_test_gl_manhattan31;
	}
	if (factory.compare("create_test_gl_manhattan31_off") == 0) {
		fn = (void*)create_test_gl_manhattan31_off;
	}
	if (factory.compare("create_test_gl_manhattan") == 0) {
		fn = (void*)create_test_gl_manhattan;
	}
	if (factory.compare("create_test_gl_manhattan_off") == 0) {
		fn = (void*)create_test_gl_manhattan_off;
	}
	if (factory.compare("create_test_gl_tess") == 0) {
		fn = (void*)create_test_gl_tess;
	}
	if (factory.compare("create_test_gl_precision") == 0) {
		fn = (void*)create_test_gl_precision;
	}
	if (factory.compare("create_test_gl_tess_off") == 0) {
		fn = (void*)create_test_gl_tess_off;
	}
	if (factory.compare("create_test_gl_trex_battery") == 0) {
		fn = (void*)create_test_gl_trex_battery;
	}
	if (factory.compare("create_test_gl_trex") == 0) {
		fn = (void*)create_test_gl_trex;
	}
	if (factory.compare("create_test_gl_trex_off") == 0) {
		fn = (void*)create_test_gl_trex_off;
	}
	if (factory.compare("create_test_gl_trex_qmatch_highp") == 0) {
		fn = (void*)create_test_gl_trex_qmatch_highp;
	}
	if (factory.compare("create_test_gl_trex_qmatch") == 0) {
		fn = (void*)create_test_gl_trex_qmatch;
	}
#endif /* __QNX__ */
	if(!fn)
	{
		if(!gfx5_handle) {
			gfx5_handle = dlopen("libgfxb_5.so", RTLD_NOW);
		}
		if(!gfx4_handle) {
			gfx4_handle = dlopen("libgfxbench40_gl.so", RTLD_NOW);
		}
		if(gfx5_handle) {
			fn = dlsym(gfx5_handle, factory.c_str());
		}
		if(gfx4_handle && !fn) {
			fn = dlsym(gfx4_handle, factory.c_str());
		}
	}

#elif defined WIN32

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	TCHAR moduleName[512];
	HANDLE process = GetCurrentProcess();
	DWORD size = 0;
	EnumProcessModules(process, NULL, 0, &size);
	size_t moduleCnt = size/sizeof(HMODULE);
	std::vector<HMODULE> modules(moduleCnt);
	EnumProcessModules(process, modules.data(), size, &size);
	for (size_t i = 0; i < modules.size(); ++i)
	{
		if (GetModuleFileNameEx(process, modules[i], moduleName, sizeof(moduleName)/sizeof(TCHAR)))
		{
			//NGLOG_DEBUG("checking module: %s", moduleName);
		}
		fn = GetProcAddress(modules[i], factory.c_str());
		if (fn != NULL)
			break;
	}
#else
#if defined _DEBUG
	HMODULE module = LoadPackagedLibrary(L"gfxbench30_dx_d", NULL);
#else
	HMODULE module = LoadPackagedLibrary(L"gfxbench30_dx", NULL);
#endif
	if (module != NULL)
	{
		fn = GetProcAddress(module, factory.c_str());
	}
	if (fn == nullptr)
	{
#if defined _DEBUG
		module = LoadPackagedLibrary(L"gfxb_5_d", NULL);
#else
		module = LoadPackagedLibrary(L"gfxb_5", NULL);
#endif
		if (module != NULL)
		{
			fn = GetProcAddress(module, factory.c_str());
		}
	}
#endif

#endif
	TestFactory f;
	f.factory_method_ = (TestFactoryFn)fn;
	return f;
}


TestBase *TestFactory::create_test()
{
	Holder holder = {0, 0};
	if (valid())
	{
		holder = factory_method_();
	}
	if (holder.test)
	{
		if (holder.version != TFW_VERSION)
		{
			NGLOG_ERROR("API version mismatch: test: %s, factory: %s", holder.version, TFW_VERSION);
			delete holder.test;
			holder.test = 0;
		}
	}
	return holder.test;
}


bool TestFactory::valid() const
{
	return factory_method_ != 0;
}


}
