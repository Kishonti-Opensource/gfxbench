/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef _WIN32
#ifdef WINGDIAPI
#undef WINGDIAPI
#endif
#ifdef APIENTRY
#undef APIENTRY
#endif
#include <windows.h>
#endif

#include <cudaw/cudaw.h>
//#define CUDAW_DEBUG
#include <cudaw/dyn_func_loader.h>

#define NVCU_INIT_POINTER(function) PROC_##function FUNC_##function = NULL;
#define NVCU_BIND_POINTER(function) FUNC_##function = bindFunctionFallbackFromV3<PROC_##function>(module, #function);

#define NVCU_INIT_POINTER_VOID(function) void* FUNC_##function = NULL;
#define NVCU_IS_POINTER_BOUND(function) (FUNC_##function != NULL) &&

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <map>

int cudawMajor(CUVersion version) { return (unsigned)version/1000; }
int cudawMinor(CUVersion version) { return ((unsigned)version%100)/10; }

class CudawHandlerObject
{
public:
	~CudawHandlerObject()
	{
		cudawRelease();
	}
};

static DYNLIB_HANDLE module = NULL;
static CUVersion highestAvailableVersion = CU_UNSET;
static CUVersion highestAvailableGLVersion = CU_UNSET;
static CudawHandlerObject object;

BoundFuncRegistry functionRegistry;

NVCU_PROCESS_3_0_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_3_1_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_3_2_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_4_0_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_4_1_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_4_2_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_5_0_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_5_5_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_6_0_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_6_5_FUNCTIONS(NVCU_INIT_POINTER)
NVCU_PROCESS_7_0_FUNCTIONS(NVCU_INIT_POINTER)

NVCU_PROCESS_3_0_GL_FUNCTIONS(NVCU_INIT_POINTER)
#ifdef _WIN32
NVCU_PROCESS_3_0_WGL_FUNCTIONS(NVCU_INIT_POINTER)
#endif
NVCU_PROCESS_4_1_GL_FUNCTIONS(NVCU_INIT_POINTER)


CUresult cudawInit(std::string* pathIn, std::string* pathOut)
{
	if (module != NULL)
		return CUDA_SUCCESS;

    std::string libStr = "lib";
    if(sizeof(void*) == 8)
    {
        libStr = "lib64";
    }
    
	std::vector< std::string > pathVec;

	if (pathIn == NULL)
	{
#ifdef _WIN32
		pathVec.push_back("nvcuda.dll");
#endif
#ifdef __QNX__
		pathVec.push_back("/usr/lib/libcuda.so.1");
#endif
#ifdef __linux__
		pathVec.push_back("libcuda.so");
        if(sizeof(void*) == 8)
        {
            pathVec.push_back("/usr/lib/x86_64-linux-gnu/libcuda.so");
        }
        else
        {
            pathVec.push_back("/usr/lib/i386-linux-gnu/libcuda.so");
        }
#endif

#ifdef __APPLE__
		pathVec.push_back("/usr/local/cuda/lib/libcuda.dylib");
#endif

#ifdef __ANDROID__
        pathVec.push_back(std::string("") + "/system/" + libStr + "/libcuda.so");
		pathVec.push_back(std::string("") + "/vendor/" + libStr + "/libcuda.so");
		pathVec.push_back(std::string("") + "/system/vendor/" + libStr + "/libcuda.so");
#endif
	}
	else
	{
		pathVec.push_back(*pathIn);
	}

	bool found = false;

	for (unsigned int i = 0; i < pathVec.size(); ++i)
	{
		module = DYNLIB_OPEN(pathVec[i].c_str());

		if (module != NULL)
		{
			NVCU_PROCESS_3_0_FUNCTIONS(NVCU_BIND_POINTER)
            
			if (cuInit != NULL && NVCU_PROCESS_3_0_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
			{
				std::cout << "[cudaw] CUDA library loaded from: " << pathVec[i] << std::endl;
				found = true;
                
                if(pathOut)
                {
                    *pathOut = pathVec[i];
                }
                
				break;
			}

			DYNLIB_CLOSE(module);
			module = NULL;
		}
	}

	if (found == false)
	{
		std::cout << "[cudaw] Could not load CUDA library" << std::endl;
		return CUDA_ERROR_NOT_FOUND;
	}
	else
	{
        NVCU_PROCESS_3_1_FUNCTIONS(NVCU_BIND_POINTER)
        NVCU_PROCESS_3_2_FUNCTIONS(NVCU_BIND_POINTER)
        NVCU_PROCESS_4_0_FUNCTIONS(NVCU_BIND_POINTER)
        NVCU_PROCESS_4_1_FUNCTIONS(NVCU_BIND_POINTER)
        NVCU_PROCESS_4_2_FUNCTIONS(NVCU_BIND_POINTER)
        NVCU_PROCESS_5_0_FUNCTIONS(NVCU_BIND_POINTER)
        NVCU_PROCESS_5_5_FUNCTIONS(NVCU_BIND_POINTER)
        NVCU_PROCESS_6_0_FUNCTIONS(NVCU_BIND_POINTER)
        NVCU_PROCESS_6_5_FUNCTIONS(NVCU_BIND_POINTER)
        NVCU_PROCESS_7_0_FUNCTIONS(NVCU_BIND_POINTER)
        
        NVCU_PROCESS_3_0_GL_FUNCTIONS(NVCU_BIND_POINTER)
#ifdef _WIN32
        NVCU_PROCESS_3_0_WGL_FUNCTIONS(NVCU_BIND_POINTER)
#endif
        NVCU_PROCESS_4_1_GL_FUNCTIONS(NVCU_BIND_POINTER)

		if (NVCU_PROCESS_3_0_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
			highestAvailableVersion = CU_3_0;
        if (NVCU_PROCESS_3_1_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_3_1;
        if (NVCU_PROCESS_3_2_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_3_2;
        if (NVCU_PROCESS_4_0_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_4_0;
        if (NVCU_PROCESS_4_1_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_4_1;
        if (NVCU_PROCESS_4_2_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_4_2;
        if (NVCU_PROCESS_5_0_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_5_0;
        if (NVCU_PROCESS_5_5_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_5_5;
        if (NVCU_PROCESS_6_0_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_6_0;
        if (NVCU_PROCESS_6_5_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_6_5;
        if (NVCU_PROCESS_7_0_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableVersion = CU_7_0;

		if (NVCU_PROCESS_3_0_GL_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
			highestAvailableGLVersion = CU_3_0;
#ifdef _WIN32
        if (NVCU_PROCESS_3_0_WGL_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableGLVersion = CU_3_0;
#endif
        if (NVCU_PROCESS_4_1_GL_FUNCTIONS(NVCU_IS_POINTER_BOUND) true)
            highestAvailableGLVersion = CU_4_1;

        if(highestAvailableVersion)
        {
            std::stringstream ss;
            ss << "[cudaw] Highest available function set: " << cudawMajor(highestAvailableVersion) << "." << cudawMinor(highestAvailableVersion);
            std::cout << ss.str() << std::endl;
        }
        else
        {
            std::cout << "[cudaw] Cannot get any version. " << std::endl;
        }

        if(highestAvailableGLVersion)
        {
            std::stringstream ss;
            ss << "[cudaw] Highest available GL function set: " << cudawMajor(highestAvailableGLVersion) << "." << cudawMinor(highestAvailableGLVersion);
            std::cout << ss.str() << std::endl;
        }
        else
        {
            std::cout << "[cudaw] Cannot get any GL version. " << std::endl;
        }

		return CUDA_SUCCESS;
	}
}

void cudawRelease()
{
	if (module)
	{
		//  ignore errors
		DYNLIB_CLOSE(module);
		module = NULL;
	}
}

bool cudawInitialized()
{
	return module != NULL;
}

CUVersion cudawHighestVersionAvailable()
{
	return highestAvailableVersion;
}

CUVersion cudawHighestGLExtensionsAvailable()
{
	return highestAvailableGLVersion;
}

void cudawHighestVersionAvailable(int* major, int* minor)
{
    *major = cudawMajor(highestAvailableVersion);
    *minor = cudawMinor(highestAvailableVersion);
}

void cudawHighestGLExtensionsAvailable(int* major, int* minor)
{
    *major = cudawMajor(highestAvailableGLVersion);
    *minor = cudawMinor(highestAvailableGLVersion);
}

#if defined(_DEBUG) || !defined(NDEBUG)

#define NVCU_PRINT_POINTER_VALUE(function) if(function != NULL) { printf("%s\t%p\n", #function, function); } else { printf("[!]%s\t%p\n", #function, function); }

void cudawPrintFunctions()
{
    printf("\n--- %s ----\n\n", "3.0");
    NVCU_PROCESS_3_0_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "3.1");
    NVCU_PROCESS_3_1_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "3.2");
    NVCU_PROCESS_3_2_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "4.0");
    NVCU_PROCESS_4_0_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "4.1");
    NVCU_PROCESS_4_1_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "4.2");
    NVCU_PROCESS_4_2_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "5.0");
    NVCU_PROCESS_5_0_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "5.5");
    NVCU_PROCESS_5_5_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "6.0");
    NVCU_PROCESS_6_0_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "6.5");
    NVCU_PROCESS_6_5_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "7.0");
    NVCU_PROCESS_7_0_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
    printf("\n--- %s ----\n\n", "3.0");
    NVCU_PROCESS_3_0_GL_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
    
#ifdef _WIN32
    printf("\n--- %s ----\n\n", "3.0");
    NVCU_PROCESS_3_0_WGL_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
#endif
    
    printf("\n--- %s ----\n\n", "4.1");
    NVCU_PROCESS_4_1_GL_FUNCTIONS(NVCU_PRINT_POINTER_VALUE)
}

void cudawPrintBoundFunctions()
{
	for (BoundFuncRegistryIt it = functionRegistry.begin(); it != functionRegistry.end(); ++it)
	{
		const std::string &name = it->first;
		const BoundFunction &func = it->second;
		const char* prefix = "[!]";
		const char* funcType = "v1";

		if (func.ptr != NULL)
			prefix = "";
		if (func.boundName.find("_v2") != func.boundName.npos)
			funcType = "v2";
		else if (func.boundName.find("_v3") != func.boundName.npos)
			funcType = "v3";
#ifndef _WIN32
		printf("%s%s\t%s\t%p\n", prefix, name.c_str(), funcType, func.ptr);
#else
		printf("%s%s\t%s\t0x%p\n", prefix, name.c_str(), funcType, func.ptr);
#endif
	}
}

#endif
