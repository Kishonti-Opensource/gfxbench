/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "CL/clew.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define VC_EXTRALEAN
    #include <windows.h>
	#include <stdlib.h>

	#define CALL_CONV _stdcall

    typedef HMODULE				DYNLIB_HANDLE;
	typedef FARPROC				PROC_ADDR_PTR;

    #define DYNLIB_OPEN			LoadLibrary
    #define DYNLIB_CLOSE		FreeLibrary
    #define DYNLIB_IMPORT		GetProcAddress
	#define REINT_CAST(ptr)		reinterpret_cast<T>(ptr)

#else
    #include <dlfcn.h>
    #include <stdlib.h>

	#define CALL_CONV

    typedef void*				DYNLIB_HANDLE;
	typedef void*				PROC_ADDR_PTR;
    #define DYNLIB_OPEN(path)	dlopen(path, RTLD_NOW | RTLD_GLOBAL)
    #define DYNLIB_CLOSE		dlclose
    #define DYNLIB_IMPORT		dlsym
	#define REINT_CAST(ptr)		reinterpret_cast<T>(reinterpret_cast<size_t>(ptr))

#endif

template<class T> T bindFunction(DYNLIB_HANDLE handle, const char* name)
{
	PROC_ADDR_PTR pointer = DYNLIB_IMPORT(handle, name);
	if (pointer == NULL)
    {
		return NULL;
	}
	return REINT_CAST(pointer);
}

#define OCLB_INIT_POINTER(function) PROC_##function FUNC_##function = NULL;
#define OCLB_BIND_POINTER(function) FUNC_##function = bindFunction<PROC_##function>(module, #function);

#define OCLB_INIT_POINTER_VOID(function) void* FUNC_##function = NULL;
#define OCLB_IS_POINTER_BOUND(function) (FUNC_##function != NULL) &&

#include <string>
#include <vector>
#include <iostream>

class ClewHandlerObject
{
public:
	~ClewHandlerObject()
	{
		clewRelease();
	}
};

static DYNLIB_HANDLE module = NULL;
static CLVersion highestAvailableVersion = CL_UNSET;
static CLVersion highestAvailableGLExtensions = CL_UNSET;
static ClewHandlerObject object;

OCLB_PROCESS_CL_1_0_FUNCTIONS(OCLB_INIT_POINTER)
OCLB_PROCESS_CL_1_1_FUNCTIONS(OCLB_INIT_POINTER)
OCLB_PROCESS_CL_1_2_FUNCTIONS(OCLB_INIT_POINTER)
OCLB_PROCESS_CL_2_0_FUNCTIONS(OCLB_INIT_POINTER)

OCLB_PROCESS_CL_1_0_GL_FUNCTIONS(OCLB_INIT_POINTER)
OCLB_PROCESS_CL_1_0_GL_KHR_FUNCTIONS(OCLB_INIT_POINTER)
OCLB_PROCESS_CL_1_2_GL_FUNCTIONS(OCLB_INIT_POINTER)

OCLB_PROCESS_CL_1_0_EGL_KHR_FUNCTIONS(OCLB_INIT_POINTER)

int clewInit(std::string* oclPathIn, std::string* oclPathOut)
{
	if (module != NULL)
		return 0;

    std::string libStr = "lib";
    if(sizeof(void*) == 8)
    {
        libStr = "lib64";
    }
    
	std::vector< std::string > clPathVec;

	if (oclPathIn == NULL)
	{
#ifdef _WIN32
		clPathVec.push_back("OpenCL.dll");
#endif

#ifdef __linux__
		clPathVec.push_back("libOpenCL.so");
		clPathVec.push_back("libOpenCL.so.1");
#endif

#ifdef __APPLE__
	#if __arm__ || __arm64__
		// Use private framework on iOS devices
		clPathVec.push_back("/System/Library/PrivateFrameworks/OpenCL.framework/OpenCL");
	#else
		clPathVec.push_back("/System/Library/Frameworks/OpenCL.framework/Versions/Current/OpenCL");
	#endif
#endif

#ifdef __ANDROID__
        clPathVec.push_back(std::string("") + "/system/" + libStr + "/libOpenCL.so");
		clPathVec.push_back(std::string("") + "/vendor/" + libStr + "/libOpenCL.so");
		clPathVec.push_back(std::string("") + "/system/vendor/" + libStr + "/libOpenCL.so");
        
        clPathVec.push_back(std::string("") + "/system/" + libStr + "/libPVROCL.so");
        clPathVec.push_back(std::string("") + "/vendor/" + libStr + "/libPVROCL.so");
        clPathVec.push_back(std::string("") + "/system/vendor/" + libStr + "/libPVROCL.so");
        
        clPathVec.push_back(std::string("") + "/system/" + libStr + "/egl/libGLES_mali.so");
        clPathVec.push_back(std::string("") + "/vendor/" + libStr + "/egl/libGLES_mali.so");
        clPathVec.push_back(std::string("") + "/system/vendor/" + libStr + "/egl/libGLES_mali.so");
#endif
	}
	else
	{
		clPathVec.push_back(*oclPathIn);
	}

	bool oclFound = false;

	for (unsigned int i = 0; i < clPathVec.size(); ++i)
	{
		module = DYNLIB_OPEN(clPathVec[i].c_str());

		if (module != NULL)
		{
			OCLB_PROCESS_CL_1_0_FUNCTIONS(OCLB_BIND_POINTER)

			if (clGetPlatformIDs && clGetDeviceIDs && clGetPlatformInfo && clGetDeviceInfo)
			{
				std::cout << "[clew] OpenCL loaded from: " << clPathVec[i] << std::endl;
				oclFound = true;
                
                if(oclPathOut)
                {
                    *oclPathOut = clPathVec[i];
                }
                
				break;
			}

			DYNLIB_CLOSE(module);
			module = 0;
		}
	}

	if (oclFound == false)
	{
		std::cout << "[clew] Could not load OpenCL" << std::endl;
		return 1;
	}
	else
	{

		OCLB_PROCESS_CL_1_1_FUNCTIONS(OCLB_BIND_POINTER)
		OCLB_PROCESS_CL_1_2_FUNCTIONS(OCLB_BIND_POINTER)
		OCLB_PROCESS_CL_2_0_FUNCTIONS(OCLB_BIND_POINTER)

		OCLB_PROCESS_CL_1_0_GL_FUNCTIONS(OCLB_BIND_POINTER)
		OCLB_PROCESS_CL_1_0_GL_KHR_FUNCTIONS(OCLB_BIND_POINTER)
		OCLB_PROCESS_CL_1_2_GL_FUNCTIONS(OCLB_BIND_POINTER)

		OCLB_PROCESS_CL_1_0_EGL_KHR_FUNCTIONS(OCLB_BIND_POINTER)

		if (OCLB_PROCESS_CL_1_0_FUNCTIONS(OCLB_IS_POINTER_BOUND) true)
			highestAvailableVersion = CL_1_0;

		if (OCLB_PROCESS_CL_1_1_FUNCTIONS(OCLB_IS_POINTER_BOUND) true)
			highestAvailableVersion = CL_1_1;

		if (OCLB_PROCESS_CL_1_2_FUNCTIONS(OCLB_IS_POINTER_BOUND) true)
			highestAvailableVersion = CL_1_2;

		if (OCLB_PROCESS_CL_2_0_FUNCTIONS(OCLB_IS_POINTER_BOUND) true)
			highestAvailableVersion = CL_2_0;

		if (OCLB_PROCESS_CL_1_0_GL_FUNCTIONS(OCLB_IS_POINTER_BOUND) true)
			highestAvailableGLExtensions = CL_1_0;

		if (OCLB_PROCESS_CL_1_2_GL_FUNCTIONS(OCLB_IS_POINTER_BOUND) true)
			highestAvailableGLExtensions = CL_1_2;
#ifdef ANDROID
        highestAvailableGLExtensions  = CL_UNSET;
		if (OCLB_PROCESS_CL_1_0_EGL_KHR_FUNCTIONS(OCLB_IS_POINTER_BOUND) true)
			highestAvailableGLExtensions = CL_1_0;
#endif
		switch (highestAvailableVersion)
		{
		case CL_1_0:
			std::cout << "[clew] Highest available: OpenCL 1.0" << std::endl;
			break;
		case CL_1_1:
			std::cout << "[clew] Highest available: OpenCL 1.1" << std::endl;
			break;
		case CL_1_2:
			std::cout << "[clew] Highest available: OpenCL 1.2" << std::endl;
			break;
		case CL_2_0:
			std::cout << "[clew] Highest available: OpenCL 2.0" << std::endl;
			break;
        case CL_UNSET:
		default:
			std::cout << "[clew] Cannot get any version. " << std::endl;
		}

        if (highestAvailableVersion != CL_UNSET)
		{
			switch (highestAvailableGLExtensions)
			{
			case CL_1_0:
				std::cout << "[clew] Highest available GL extensions: 1.0" << std::endl;
				break;
			case CL_1_2:
				std::cout << "[clew] Highest available GL extensions: 1.2" << std::endl;
				break;
            case CL_UNSET:
			default:
				std::cout << "[clew] Cannot get any GL version. " << std::endl;
			}
		}

		return 0;
	}
}

int clewRelease()
{
	if (module)
	{
		//  ignore errors
		DYNLIB_CLOSE(module);
		module = 0;
	}

	return 0;
}

bool clewInitialized()
{
	return module != NULL;
}

CLVersion clewHighestVersionAvailable()
{
	return highestAvailableVersion;
}

CLVersion clewHighestGLExtensionsAvailable()
{
	return highestAvailableGLExtensions;
}

void clewHighestVersionAvailable(int* major, int* minor)
{
	switch (highestAvailableVersion)
	{
	case CL_1_0:
		*major = 1; *minor = 0;
		break;
	case CL_1_1:
		*major = 1; *minor = 1;
		break;
	case CL_1_2:
		*major = 1; *minor = 2;
		break;
	case CL_2_0:
		*major = 2; *minor = 0;
		break;
	default:
		*major = 0; *minor = 0;
	}
}

void clewHighestGLExtensionsAvailable(int* major, int* minor)
{
	switch (highestAvailableGLExtensions)
	{
	case CL_1_0:
		*major = 1; *minor = 0;
		break;
	case CL_1_2:
		*major = 1; *minor = 2;
		break;
	default:
		*major = 0; *minor = 0;
	}
}
