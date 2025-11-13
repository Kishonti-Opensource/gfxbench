/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "TestMode.h"
#include <cstdint>
#include <iostream>
#include <sstream>

#include "ng/log.h"

#define _WIN32_DCOM
#define NOMINMAX
#include <windows.h>
#include <comdef.h>
#include <atlstr.h>
#include <strsafe.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
 
using namespace std;

UINT timeoutLowPower = 0, timeoutPowerOff = 0, timeoutScreenSaver = 0;

REASON_CONTEXT reasonContext;
HANDLE powerHandle = 0;

typedef struct BrightnessData
{
	BrightnessData(): level(-1), min(-1), max(-1), levelCount(0), levelArray(NULL) {}
	BrightnessData(unsigned level_): level(level_), min(-1), max(-1), levelCount(0), levelArray(NULL) {}
	BrightnessData(const BrightnessData &other)
	{
		if(other.levelCount)
		{
			allocLevelArray(other.levelCount);
			levelCount = other.levelCount;
			for(size_t i(0); i < levelCount; i++)
			{
				levelArray[i] = other.levelArray[i];
			}
		}
		level = other.level;
		min = other.min;
		max = other.max;
	}
	BrightnessData& operator=(const BrightnessData &other)
	{
		if(this == &other)
		{
			return *this;
		}
		freeLevelArray();
		if(other.levelCount)
		{
			allocLevelArray(other.levelCount);
			levelCount = other.levelCount;
			for(size_t i(0); i < levelCount; i++)
			{
				levelArray[i] = other.levelArray[i];
			}
		}
		level = other.level;
		min = other.min;
		max = other.max;
		return *this;
	}
	~BrightnessData()
	{
		freeLevelArray();
	}

	double percent()
	{
		return (double)level / max;
	}

	void setPercent(double percent)
	{
		if(percent > 1.0)
			percent = 1.0;
		else if(percent < 0.0)
			percent = 0.0;
		double diff = max * percent;
		unsigned val = static_cast<unsigned>(floor(diff+0.5));
		level = -1;
		for(size_t i(0); i < levelCount; i++)
		{
			if(levelArray[i] >= val)
			{
				level = val;
				break;
			}
		}
		if(level == -1)
		{
			level = max;
		}
	}

	bool isValid()
	{
		return level >= 0 && min >= 0 && max >= 0 &&
			min <= level && level <= max;
	}

	void allocLevelArray(size_t count)
	{
		freeLevelArray();
		levelArray = new unsigned[count];
	}

	void freeLevelArray()
	{
		if(levelArray)
		{
			delete[] levelArray;
			levelArray = NULL;
			levelCount = 0;
		}
	}
	int level, min, max;

	unsigned *levelArray;
	size_t levelCount;

} BDATA, *PBDATA;

BrightnessData savedBrightness;
bool monitorFlag = false;
int monitorCount = 0;
int nameLength[10];

void createHiddenConsole()
{
    if (!AllocConsole()) {
        NGLOG_WARN("[TestMode]: cannot alloc hidden console");
        return;
    }
    HWND hwnd = GetConsoleWindow();
    if (!hwnd) {
        NGLOG_WARN("[TestMode]: cannot get console window");
        return;
    }
	ShowWindow(hwnd, SW_HIDE);
}

void deleteHiddenConsole()
{	
	if(!FreeConsole()) {
        NGLOG_WARN("[TestMode]: cannot release hidden console");
	}
}

int ThreadStarter(LPTHREAD_START_ROUTINE pFunction, PBDATA inData = NULL, PBDATA outData = NULL)
{
	PBDATA pData;
	DWORD dwThreadId;
	HANDLE hThread;

	pData = (PBDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BrightnessData));
	if(!pData)
	{
		return 1;
	}
	if(inData)
	{
		*pData = *inData;
	}

	hThread = CreateThread(NULL, 0, pFunction, pData, 0, &dwThreadId);
	if(!hThread)
	{
		return 1;
	}

	WaitForSingleObject(hThread, INFINITE);

	if(outData)
	{
		*outData = *pData;
	}

	CloseHandle(hThread);
	if(pData)
	{
		HeapFree(GetProcessHeap(), 0, pData);
		pData = NULL;
	}
	return 0;
}

int ThreadStarter(LPTHREAD_START_ROUTINE pFunction, int* pInOut = NULL)
{
	int *pInt;
	DWORD dwThreadId;
	HANDLE hThread;

	pInt = (int*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 10*sizeof(int));
	if (!pInt)
	{
		return 1;
	}

	memcpy(pInt, pInOut, 10 * sizeof(int));
	hThread = CreateThread(NULL, 0, pFunction, pInt, 0, &dwThreadId);
	if (!hThread)
	{
		return 1;
	}

	WaitForSingleObject(hThread, INFINITE);

	if (pInOut)
	{
		memcpy(pInOut, pInt, 10 * sizeof(int));
	}

	CloseHandle(hThread);
	if (pInt)
	{
		HeapFree(GetProcessHeap(), 0, pInt);
		pInt = NULL;
	}
	return 0;
}

DWORD WINAPI GetBrightnessThreadFunction(LPVOID pData)
{
	HRESULT hres;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		cout << "Failed to initialize COM library. Error code = 0x"
			<< hex << hres << endl;
		return 1;                  // Program has failed.
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------

	//hres =  CoInitializeSecurity(
	//    NULL, 
	//    -1,                          // COM authentication
	//    NULL,                        // Authentication services
	//    NULL,                        // Reserved
	//    RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
	//    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
	//    NULL,                        // Authentication info
	//    EOAC_NONE,                   // Additional capabilities 
	//    NULL                         // Reserved
	//    );

	//                  
	//if (FAILED(hres))
	//{
	//    cout << "Failed to initialize security. Error code = 0x" 
	//        << hex << hres << endl;
	//    CoUninitialize();
	//    return 1;                    // Program has failed.
	//}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres))
	{
		cout << "Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< hex << hres << endl;
		CoUninitialize();
		return 1;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\WMI"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
		);

	if (FAILED(hres))
	{
		cout << "Could not connect. Error code = 0x"
			<< hex << hres << endl;
		pLoc->Release();
		CoUninitialize();
		return 1;                // Program has failed.
	}

	cout << "Connected to ROOT\\WMI namespace" << endl;


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
		);

	if (FAILED(hres))
	{
		cout << "Could not set proxy blanket. Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM WmiMonitorBrightness"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		cout << "Query for operating system name failed."
			<< " Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject *pclsObj = 0;
	ULONG uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;

		// Get the value of the Name property
		hr = pclsObj->Get(L"CurrentBrightness", 0, &vtProp, 0, 0);
		VariantClear(&vtProp);

		PBDATA pbData = (PBDATA)pData;
		pbData->level = static_cast<unsigned>(vtProp.ullVal);

		hr = pclsObj->Get(L"Level", 0, &vtProp, 0, 0);
		long lUpper = 0;
		long lLower = 0;
		hr = SafeArrayGetUBound(V_ARRAY(&vtProp), 1, &lUpper);
		hr = SafeArrayGetLBound(V_ARRAY(&vtProp), 1, &lLower);
		long lNumOfElems = lUpper - lLower + 1;
		BYTE HUGEP *plongArray;
		hr = SafeArrayAccessData(V_ARRAY(&vtProp), (void**)&plongArray);

		pbData->min = plongArray[0];
		pbData->max = plongArray[lNumOfElems - 1];
		pbData->allocLevelArray(lNumOfElems);
		pbData->levelCount = lNumOfElems;
		for (int i = 0; i < lNumOfElems; i++)
		{
			pbData->levelArray[i] = plongArray[i];
		}
		VariantClear(&vtProp);

		pclsObj->Release();
		pclsObj = NULL;
	}

	// Cleanup
	// ========

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();

	if (pclsObj) pclsObj->Release();
	CoUninitialize();

	return 0;   // Program successfully completed.
}


DWORD WINAPI SetBrightnessThreadFunction(LPVOID pData)
{
	HRESULT hres;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		cout << "Failed to initialize COM library. Error code = 0x"
			<< hex << hres << endl;
		return 1;                  // Program has failed.
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------

	//hres =  CoInitializeSecurity(
	//    NULL, WmiSetBrightness
	//    -1,                          // COM authentication
	//    NULL,                        // Authentication services
	//    NULL,                        // Reserved
	//    RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
	//    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
	//    NULL,                        // Authentication info
	//    EOAC_NONE,                   // Additional capabilities 
	//    NULL                         // Reserved
	//    );

	//                  
	//if (FAILED(hres))
	//{
	//    cout << "Failed to initialize security. Error code = 0x" 
	//        << hex << hres << endl;
	//    CoUninitialize();
	//    return 1;                    // Program has failed.
	//}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres))
	{
		cout << "Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< hex << hres << endl;
		CoUninitialize();
		return 1;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer( // WMI   CIMV2
		_bstr_t(L"ROOT\\WMI"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
		);

	if (FAILED(hres))
	{
		cout << "Could not connect. Error code = 0x"
			<< hex << hres << endl;
		pLoc->Release();
		CoUninitialize();
		return 1;                // Program has failed.
	}

	//cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
		);

	if (FAILED(hres))
	{
		cout << "Could not set proxy blanket. Error code = 0x"
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	IEnumWbemClassObject *pEnum = NULL;

	hres = pSvc->ExecQuery(_bstr_t(L"WQL"), //Query Language
		L"Select * from WmiMonitorBrightnessMethods", //Query to Execute
		WBEM_FLAG_RETURN_IMMEDIATELY, //Make a semi-synchronous call
		NULL, //Context
		&pEnum //Enumeration Interface
		);


	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	ULONG ulReturned;
	IWbemClassObject *pObj;

	//Get the Next Object from the collection
	hres = pEnum->Next(WBEM_INFINITE, //Timeout
		1, //No of objects requested
		&pObj, //Returned Object
		&ulReturned //No of object returned
		);

	// set up to call the Win32_Process::Create method
	BSTR MethodName = SysAllocString(L"WmiSetBrightness");
	BSTR ClassName = SysAllocString(L"WmiMonitorBrightnessMethods");

	IWbemClassObject* pClass = NULL;
	hres = pSvc->GetObject(ClassName, 0, NULL, &pClass, NULL);

	IWbemClassObject* pInParamsDefinition = NULL;
	hres = pClass->GetMethod(MethodName, 0, &pInParamsDefinition, NULL);

	IWbemClassObject* pClassInstance = NULL;
	hres = pInParamsDefinition->SpawnInstance(0, &pClassInstance);

	// set timeout value
	VARIANT var1;
	VariantInit(&var1);

	V_VT(&var1) = VT_BSTR;
	V_BSTR(&var1) = SysAllocString(L"0");
	hres = pClassInstance->Put(L"Timeout", 0, &var1, CIM_UINT32); //CIM_UINT64

	if (hres != WBEM_S_NO_ERROR)
	{
		VariantClear(&var1);
		SysFreeString(ClassName);
		SysFreeString(MethodName);
		pClass->Release();
		pInParamsDefinition->Release();
		pLoc->Release();
		pSvc->Release();
		CoUninitialize();
		cout << "Cannot put parameter 'Timeout'" << endl;
		return 1;
	}

	// set brightness value
	VARIANT var2;
	VariantInit(&var2);

	PBDATA pbData = (PBDATA)pData;
	std::stringstream ss;
	ss << pbData->level;
	CString cstring(ss.str().c_str());
	V_VT(&var2) = VT_BSTR;
	V_BSTR(&var2) = cstring.AllocSysString();
	hres = pClassInstance->Put(L"Brightness", 0, &var2, CIM_UINT8);
	
	if (hres != WBEM_S_NO_ERROR)
	{
		VariantClear(&var1);
		VariantClear(&var2);
		SysFreeString(ClassName);
		SysFreeString(MethodName);
		pClass->Release();
		pInParamsDefinition->Release();
		pLoc->Release();
		pSvc->Release();
		CoUninitialize();
		cout << "Cannot put parameter 'Brightness'" << endl;
		return 1;
	}

	VARIANT pathVariable;
	VariantInit(&pathVariable);

	hres = pObj->Get(_bstr_t(L"__PATH"), 0, &pathVariable, NULL, NULL);
	if (hres != WBEM_S_NO_ERROR)
	{
		VariantClear(&var1);
		VariantClear(&var2);
		SysFreeString(ClassName);
		SysFreeString(MethodName);
		pClass->Release();
		pInParamsDefinition->Release();
		pLoc->Release();
		pSvc->Release();
		CoUninitialize();
		cout << "Cannot get parameter 'Path'" << endl;
		return 1;
	}

	// Execute Method
	hres = pSvc->ExecMethod(pathVariable.bstrVal, MethodName, 0, NULL, pClassInstance, NULL, NULL);
	VariantClear(&pathVariable);
	if (hres != WBEM_S_NO_ERROR)
	{
		VariantClear(&var1);
		VariantClear(&var2);
		SysFreeString(ClassName);
		SysFreeString(MethodName);
		pClass->Release();
		pInParamsDefinition->Release();
		pLoc->Release();
		pSvc->Release();
		CoUninitialize();
		cout << "Cannot exec method 'WmiSetBrightness'" << endl;
		return 1;
	}


	if (FAILED(hres))
	{
		cout << "Could not execute method. Error code = 0x"
			<< hex << hres << endl;

		VariantClear(&var1);
		VariantClear(&var2);
		SysFreeString(ClassName);
		SysFreeString(MethodName);
		pClass->Release();
		pInParamsDefinition->Release();
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// To see what the method returned,
	// use the following code.  The return value will
	// be in &varReturnValue

	
	// Clean up
	//--------------------------
	VariantClear(&var1);
	VariantClear(&var2);
	SysFreeString(ClassName);
	SysFreeString(MethodName);
	pClass->Release();
	pInParamsDefinition->Release();
	pLoc->Release();
	pSvc->Release();
	CoUninitialize();

	return 0;
}

TestMode::TestMode()
{
	m_brightnessChanged = false;
	m_testModeIsOn = false;
}

TestMode::~TestMode()
{
	leave();
}

void TestMode::setBrightness(double value)
{
	BrightnessData setBrightnessData;
	setBrightnessData = savedBrightness;
	setBrightnessData.setPercent(value);
	ThreadStarter(SetBrightnessThreadFunction, &setBrightnessData, 0);
	instance()->m_brightnessChanged = true;
}

void TestMode::restoreBrightness()
{
	if(instance()->m_brightnessChanged)
	{
		setBrightness(savedBrightness.percent());
		instance()->m_brightnessChanged = false;
	}
}

void TestMode::setSleepEnabled(bool enable)
{
    if (enable) {
        SetThreadExecutionState(ES_CONTINUOUS);
    } else {
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
    }
}

void TestMode::saveBrightness()
{
	ThreadStarter(GetBrightnessThreadFunction, 0, &savedBrightness);
}
