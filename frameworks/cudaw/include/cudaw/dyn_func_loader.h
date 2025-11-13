/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DYN_FUNC_LOADER_H
#define DYN_FUNC_LOADER_H

#include <map>
#include <string>

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

#if defined(_DEBUG) || !defined(NDEBUG)
#define REG_FUNCTION(name, boundName, ptr) functionRegistry.insert(std::pair<std::string, BoundFunction>(std::string(name), BoundFunction(std::string(boundName), ptr)));
#else
#define REG_FUNCTION(name, boundName, ptr)
#endif

struct BoundFunction
{
	std::string boundName;
	PROC_ADDR_PTR ptr;

	BoundFunction(std::string boundName, PROC_ADDR_PTR ptr)
	{
		this->boundName = boundName;
		this->ptr = ptr;
	}
};

typedef std::map<const std::string, const BoundFunction> BoundFuncRegistry;
typedef BoundFuncRegistry::iterator BoundFuncRegistryIt;

extern BoundFuncRegistry functionRegistry;

template<class T> T bindFunction(DYNLIB_HANDLE handle, const char* name)
{
	PROC_ADDR_PTR pointer = DYNLIB_IMPORT(handle, name);
	if (pointer == NULL)
    {
		REG_FUNCTION(name, name, pointer);
		return NULL;
	}
	REG_FUNCTION(name, name, pointer);
	return REINT_CAST(pointer);
}

#ifdef _WIN32
#define c_strcpy strcpy_s
#define c_strcat strcat_s
#else
#include <cstring>
#define c_strcpy strcpy
#define c_strcat strcat
#endif

template<class T> T bindFunctionFallbackFromV3(DYNLIB_HANDLE handle, const char* name)
{
	char name_v2[128];
	c_strcpy(name_v2, name);
	c_strcat(name_v2, "_v2");

	char name_v3[128];
	c_strcpy(name_v3, name);
	c_strcat(name_v3, "_v3");
    PROC_ADDR_PTR pointer;
    //v3
    pointer = DYNLIB_IMPORT(handle, name_v3);
    if (pointer != NULL)
    {
		REG_FUNCTION(name, name_v3, pointer);
        return REINT_CAST(pointer);
    }
    //v2
    pointer = DYNLIB_IMPORT(handle, name_v2);
    if (pointer != NULL)
    {
		REG_FUNCTION(name, name_v2, pointer);
        return REINT_CAST(pointer);
    }
    //v1
    pointer = DYNLIB_IMPORT(handle, name);
    if (pointer != NULL)
    {
		REG_FUNCTION(name, name, pointer);
        return REINT_CAST(pointer);
    }
	REG_FUNCTION(name, name, pointer);
    return NULL;
}

#endif
