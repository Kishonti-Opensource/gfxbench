/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#define _WIN32_DCOM

#include "windowsdeviceinfocollector.h"
#include "utils.h"

#include <windows.h>
#include <PowrProf.h>
#include <intrin.h>
#include <sstream>
#include <WbemCli.h>
#include <winternl.h>
#include <tchar.h>
#include <comdef.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>

#if defined HAVE_GLEW
#include <GL/glew.h>
#else
#include <GL/gl.h>
#endif

#include <dshow.h>
#include <Dvdmedia.h>

using namespace std;
#include <Wbemidl.h>

#pragma warning(push)
#pragma warning(disable:4068)
#include <BluetoothAPIs.h>
#pragma warning(pop)
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Bthprops.lib")

#pragma comment(lib, "strmiids")
#pragma comment(lib, "wbemuuid.lib")

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
typedef UINT (WINAPI* GETSYSTEMWOW64DIRECTORY)(LPTSTR, UINT);

#define PRODUCT_PROFESSIONAL	0x00000030
#define VER_SUITE_WH_SERVER	0x00008000

#define D3D_DEBUG_INFO
#include <D3D9.h>
#pragma comment(lib,"d3d9.lib")

#ifndef _M_ARM64
#include "nvapi.h"
#include "amd_ags.h"
#endif

using namespace sysinf;

namespace WMI
{
	class WmiQuery
	{
	private:
		IWbemLocator* m_locator;
		IWbemServices* m_service;
		IEnumWbemClassObject* m_enumerator;
		IWbemClassObject* m_classObject;
		HRESULT m_hr;

	public:
		// http://msdn.microsoft.com/en-us/library/aa390423(v=vs.85).aspx
		WmiQuery(const BSTR strNamespace, const BSTR strQuery) :
			m_locator(NULL),
			m_service(NULL),
			m_classObject(NULL),
			m_enumerator(NULL)
		{
			// Step 1: --------------------------------------------------
			// Initialize COM. ------------------------------------------

			m_hr =  CoInitializeEx(0, COINIT_MULTITHREADED);
			if (FAILED(m_hr))
			{
				cout << "Failed to initialize COM library. Error code = 0x"
					<< hex << m_hr << endl;
				return;                  // Program has failed.
			}

			// Step 2: --------------------------------------------------
			// Set general COM security levels --------------------------
			/*
			m_hr =  CoInitializeSecurity(
				NULL,
				-1,                          // COM authentication
				NULL,                        // Authentication services
				NULL,                        // Reserved
				RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
				RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
				NULL,                        // Authentication info
				EOAC_NONE,                   // Additional capabilities
				NULL                         // Reserved
				);


			if (FAILED(m_hr))
			{
				cout << "Failed to initialize security. Error code = 0x" << hex << m_hr << endl;
				return;                    // Program has failed.
			}
			*/
			// Step 3: ---------------------------------------------------
			// Obtain the initial locator to WMI -------------------------

			m_hr = CoCreateInstance(
				CLSID_WbemLocator,
				0,
				CLSCTX_INPROC_SERVER,
				IID_IWbemLocator, (LPVOID *) &m_locator);

			if (FAILED(m_hr))
			{
				cout << "Failed to create IWbemLocator object."
					<< " Err code = 0x"
					<< hex << m_hr << endl;
				return;                 // Program has failed.
			}

			// Step 4: -----------------------------------------------------
			// Connect to WMI through the IWbemLocator::ConnectServer method

			// Connect to the root\cimv2 namespace with
			// the current user and obtain pointer pSvc
			// to make IWbemServices calls.
			m_hr = m_locator->ConnectServer(
				 _bstr_t(strNamespace), // Object path of WMI namespace
				 NULL,                    // User name. NULL = current user
				 NULL,                    // User password. NULL = current
				 0,                       // Locale. NULL indicates current
				 NULL,                    // Security flags.
				 0,                       // Authority (for example, Kerberos)
				 0,                       // Context object
				 &m_service               // pointer to IWbemServices proxy
				 );

			if (FAILED(m_hr))
			{
				cout << "Could not connect. Error code = 0x"
					 << hex << m_hr << endl;
				return;                // Program has failed.
			}

			cout << "Connected to WMI namespace" << endl;


			// Step 5: --------------------------------------------------
			// Set security levels on the proxy -------------------------

			m_hr = CoSetProxyBlanket(
			   m_service,                        // Indicates the proxy to set
			   RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
			   RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
			   NULL,                        // Server principal name
			   RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
			   RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
			   NULL,                        // client identity
			   EOAC_NONE                    // proxy capabilities
			);

			if (FAILED(m_hr))
			{
				cout << "Could not set proxy blanket. Error code = 0x"
					<< hex << m_hr << endl;
				return;               // Program has failed.
			}

			// Step 6: --------------------------------------------------
			// Use the IWbemServices pointer to make requests of WMI ----

			// For example, get the name of the operating system

			m_hr = m_service->ExecQuery(
				bstr_t("WQL"),
				bstr_t(strQuery),
				WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
				NULL,
				&m_enumerator);

			if (FAILED(m_hr))
			{
				cout << "Query failed."
					<< " Error code = 0x"
					<< hex << m_hr << endl;
				return;               // Program has failed.
			}

			// Step 7: -------------------------------------------------
			// Get the data from the query in step 6 -------------------

			/*ULONG uReturn = 0;

			while (pEnumerator)
			{
				HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
					&m_classObject, &uReturn);

				if(0 == uReturn)
				{
					break;
				}

				VARIANT vtProp;

				//Get the value of the Name property
				std::wstring req = L"Model";
				hr = m_classObject->Get(req.c_str(), 0, &vtProp, 0, 0);
				wcout << req << " : " << vtProp.bstrVal << endl;
				VariantClear(&vtProp);
			}*/

			/*
			using std::cout;
			using std::cin;
			using std::endl;

			if(FAILED(m_hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
			{
				cout << "Unable to launch COM: 0x" << std::hex << m_hr << std::dec << endl;
				return;
			}

			m_hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
			if(FAILED(m_hr) && (m_hr != RPC_E_TOO_LATE))
			{
				cout << "Unable to initialize security: 0x" << std::hex << m_hr << std::dec << endl;
				return;
			}

			if(FAILED(m_hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_ALL, IID_PPV_ARGS(&m_locator))))
			{
				cout << "Unable to create a WbemLocator: " << std::hex << m_hr << std::dec << endl;
				return;
			}

			if(FAILED(m_hr = m_locator->ConnectServer(L"root\\CIMV2", NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &m_service)))
			{
				cout << "Unable to connect to \"CIMV2\": " << std::hex << m_hr << std::dec << endl;
				return;
			}

			if(FAILED(m_hr = m_service->ExecQuery(L"WQL", strQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &m_enumerator)))
			{
				cout << "Unable to ExecQuery: " << std::hex << m_hr << std::dec << endl;
				return;
			}	*/
		}

		~WmiQuery()
		{
			ReleaseAll();
			CoUninitialize();
		}

		inline HRESULT GetError() const
		{
			return m_hr;
		}

		IWbemClassObject* GetCurrent() const
		{
			return m_classObject;
		}

		IWbemClassObject* GetNext()
		{
			if (!m_enumerator)
			{
				return NULL;
			}

			if (m_classObject != NULL)
			{
				m_classObject->Release();
				m_classObject = NULL;
			}

			ULONG fetched;

			m_hr = m_enumerator->Next(WBEM_INFINITE, 1, &m_classObject, &fetched);

			return ((m_hr == WBEM_S_FALSE) || FAILED(m_hr)) ? NULL : m_classObject;
		}

		void ReleaseAll()
		{
			if (m_locator)
			{
				m_locator->Release();
				m_locator = NULL;
			}

			if (m_service)
			{
				m_service->Release();
				m_service = NULL;
			}

			if (m_enumerator)
			{
				m_enumerator->Release();
				m_enumerator = NULL;
			}

			if (m_classObject)
			{
				m_classObject->Release();
				m_classObject = NULL;
			}
		}
	};
}

namespace
{
	inline bool GetString(IPropertyBag *pPropBag, LPCOLESTR pszPropName, std::string* dst)
	{
		VARIANT vRet;
		VariantInit(&vRet);
		if (SUCCEEDED(pPropBag->Read(pszPropName, &vRet, 0)) && vRet.vt == VT_BSTR)
		{
			*dst = bstr_t(vRet.bstrVal, true);
			VariantClear(&vRet);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool GetString(IWbemClassObject *clsObj, LPCWSTR wszName, std::string* dst)
	{
		VARIANT vRet;
		VariantInit(&vRet);
		if (SUCCEEDED(clsObj->Get(wszName, 0, &vRet, NULL, NULL)) && vRet.vt == VT_BSTR)
		{
			*dst = bstr_t(vRet.bstrVal, true);
			VariantClear(&vRet);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool GetBool(IWbemClassObject *clsObj, LPCWSTR wszName, bool* dst)
	{
		VARIANT vRet;
		VariantInit(&vRet);
		if (SUCCEEDED(clsObj->Get(wszName, 0, &vRet, NULL, NULL)) && vRet.vt == VT_BOOL)
		{
			*dst = vRet.boolVal != 0;
			VariantClear(&vRet);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool GetUInt32(IWbemClassObject *clsObj, LPCWSTR wszName, UINT32* dst)
	{
		VARIANT vRet;
		VariantInit(&vRet);
		if (SUCCEEDED(clsObj->Get(wszName, 0, &vRet, NULL, NULL)) && vRet.vt == VT_UI4)
		{
			*dst = vRet.ulVal;
			VariantClear(&vRet);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool GetInt32(IWbemClassObject *clsObj, LPCWSTR wszName, INT32* dst)
	{
		VARIANT vRet;
		VariantInit(&vRet);
		if (SUCCEEDED(clsObj->Get(wszName, 0, &vRet, NULL, NULL)) && vRet.vt == VT_I4)
		{
			*dst = vRet.lVal;
			VariantClear(&vRet);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool GetUInt64(IWbemClassObject *clsObj, LPCWSTR wszName, UINT64* dst)
	{
		VARIANTARG vRet;
		VariantInit(&vRet);
		if (SUCCEEDED(clsObj->Get(wszName, 0, &vRet, NULL, NULL)))
		{
			switch (vRet.vt)
			{
				case VT_BSTR:
					{
						std::string value =  bstr_t(vRet.bstrVal, true);
						*dst = atol(value.c_str());
						return true;
					}

				case VT_UI8:
					{
						*dst = vRet.ullVal;
						return true;
					}
				default:
					return false;
			}
		}
		return false;
	}

    std::map<std::string, std::string> getFields(IWbemClassObject* clsObj)
    {
        std::map<std::string, std::string> result;
        clsObj->BeginEnumeration(0);
        BSTR name;
        VARIANT value;
        while (clsObj->Next(0, &name, &value, nullptr, nullptr) == WBEM_S_NO_ERROR) {
            std::string nameString = _bstr_t(name, true);
            if ((nameString.substr(0, 2) != "__") &&
                (nameString != "SystemName") &&
                (nameString != "AdapterRAM") && //deprecated
                SUCCEEDED(VariantChangeType(&value, &value, 0, VT_BSTR)))
            {
                result[nameString] = _bstr_t(value);
            }
        }
        clsObj->EndEnumeration();
        return result;
    }
}

std::string eraseWhiteSpaces(std::string str)
{
	size_t index0;
    size_t index1;
	index0 = str.find_first_not_of(' ');
    index1 = str.find_last_not_of(' ');
	return str.substr(index0, index1);
}

bool TestTopologyLevel(int* pInfo, int nMode, int nLevel)
{
#ifndef _M_ARM64
    __cpuidex(pInfo,0xB,nMode);
    return ((pInfo[2] >> 8) & 0xFF) == nLevel;
#else
    return false;
#endif
}

int GetCoreCount(int* nInfo)
{
#ifndef _M_ARM64
    __cpuid(nInfo,0);
    if(nInfo[0] >= 4)
    {
        __cpuidex(nInfo,0x4,0x0);
        return (nInfo[0] >> 26) + 1;
    }
    else
    {
        __cpuid(nInfo,0x80000000);
        int nCPUID = nInfo[0];
        __cpuid(nInfo,1);
        if(nInfo[3] & 0x10000000 && nCPUID >= 0x80000008)
        {
            __cpuid(nInfo,0x80000008);
            return (nInfo[2] & 0xFF) + 1;
            }
        }
    return 1;
#else
    return 0;
#endif
}

BOOL IsWOW64(void)
{
#ifdef _WIN64
	return FALSE;

#else
	GETSYSTEMWOW64DIRECTORY getSystemWow64Directory;
	HMODULE hKernel32;
	TCHAR Wow64Directory[MAX_PATH];

	hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
	if (hKernel32 == NULL) {
	//
	// This shouldn't happen, but if we can't get
	// kernel32's module handle then assume we are
	//on x86. We won't ever install 32-bit drivers
	// on 64-bit machines, we just want to catch it
	// up front to give users a better error message.
	//
	return FALSE;
	}

	getSystemWow64Directory = (GETSYSTEMWOW64DIRECTORY)
	GetProcAddress(hKernel32, "GetSystemWow64DirectoryW");

	if (getSystemWow64Directory == NULL) {
	//
	// This most likely means we are running
	// on Windows 2000, which didn't have this API
	// and didn't have a 64-bit counterpart.
	//
	return FALSE;
	}

	if ((getSystemWow64Directory(Wow64Directory,
	sizeof(Wow64Directory)/sizeof(TCHAR)) == 0) &&
	(GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)) {
	return FALSE;
	}

	//
	// GetSystemWow64Directory succeeded
	// so we are on a 64-bit OS.
	//
	return TRUE;
#endif
}

void GetProcessorTopology(int& nPhysical, int& nCores, int& nSystemThreads, int& nHWThreads, int& nThreadsPerCore)
{
    int nInfo[4];
    SYSTEM_INFO pSystemInfo;
    IsWOW64() ? GetNativeSystemInfo(&pSystemInfo) : GetSystemInfo(&pSystemInfo);
    nSystemThreads = pSystemInfo.dwNumberOfProcessors;
    if(TestTopologyLevel(nInfo,0x0,1) || TestTopologyLevel(nInfo,0x1,1))
    {
        nThreadsPerCore = nInfo[1] & 0xFFFF;
        if(TestTopologyLevel(nInfo,0x0,2) || TestTopologyLevel(nInfo,0x1,2))
        {
            nHWThreads = nInfo[1] & 0xFFFF;
            nCores = (nThreadsPerCore == 0) ? GetCoreCount(nInfo) : nHWThreads / nThreadsPerCore;
        }
        else
        {
            nHWThreads = nSystemThreads;
            nCores = GetCoreCount(nInfo);
        }
    }
    else
    {
        nThreadsPerCore = 1;
        nHWThreads = nSystemThreads;
        nCores = GetCoreCount(nInfo);
    }

    nPhysical = (nCores == 0) ? 1 : (nSystemThreads / nThreadsPerCore) / nCores;
}

uint64_t GetMeasuredProcessorFreq()
{
#ifndef _M_ARM64
    LARGE_INTEGER qwWait, qwStart, qwCurrent;
    QueryPerformanceCounter(&qwStart);
    QueryPerformanceFrequency(&qwWait);
    qwWait.QuadPart >>= 5;
    unsigned __int64 Start = __rdtsc();
    do
    {
        QueryPerformanceCounter(&qwCurrent);
    }while(qwCurrent.QuadPart - qwStart.QuadPart < qwWait.QuadPart);
    return ((__rdtsc() - Start) << 5);
#else
    return 0;
#endif
}

BOOL CALLBACK MonitorInfoEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	std::vector<MONITORINFOEX>* monitors = (std::vector<MONITORINFOEX>*)dwData;

    MONITORINFOEX mi;
    ZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);

    GetMonitorInfo(hMonitor, &mi);
	monitors->push_back(mi);

    return TRUE;
}

//System manufacturer - System Product Name
struct DeviceDesc
{
	std::string	Manufacturer;
	std::string Model;
	std::string SystemType;
	DeviceDesc() : Manufacturer("N/A"), Model("N/A"), SystemType("N/A") {}
};

DeviceDesc GetDeviceName()
{
	using std::cout;
	using std::cin;
	using std::endl;

	DeviceDesc device;
	WMI::WmiQuery deviceNameQuery(L"ROOT\\CIMV2",L"SELECT Manufacturer, Model, SystemType FROM Win32_ComputerSystem");
	if (FAILED(deviceNameQuery.GetError())) {
		device.Manufacturer = "error";
		device.Model = "error";
		device.SystemType = "error";
		return device;
	}

    IWbemClassObject* clsObj = NULL;
	std::string manufacturer;
    std::string model;
	while ((clsObj = deviceNameQuery.GetNext()) != NULL)
	{
		VARIANT vRet;
		VariantInit(&vRet);

        if(SUCCEEDED(clsObj->Get(L"Manufacturer", 0, &vRet, NULL, NULL)) && vRet.vt == VT_BSTR)
		{
			device.Manufacturer = _bstr_t(vRet.bstrVal);
			VariantClear(&vRet);
		}

        if(SUCCEEDED(clsObj->Get(L"Model", 0, &vRet, NULL, NULL)) && vRet.vt == VT_BSTR)
		{
			device.Model = _bstr_t(vRet.bstrVal);
			VariantClear(&vRet);
		}

		if(SUCCEEDED(clsObj->Get(L"SystemType", 0, &vRet, NULL, NULL)) && vRet.vt == VT_BSTR)
		{
			device.SystemType = _bstr_t(vRet.bstrVal);
			VariantClear(&vRet);
		}
	}

	if(device.Model == "System Product Name")
		device.Model = device.SystemType;

	return device;
}

typedef NTSTATUS(WINAPI* _NtQueryInformationProcess) (_In_ HANDLE ProcessHandle, _In_ PROCESSINFOCLASS ProcessInformationClass, _Out_ PVOID ProcessInformation, _In_ ULONG ProcessInformationLength, _Out_opt_ PULONG ReturnLength);
_NtQueryInformationProcess NtQueryInformationProcess_;


size_t GetProcessPEBAddress(HANDLE hProc)
{
	PROCESS_BASIC_INFORMATION peb;
	DWORD tmp;
	NtQueryInformationProcess_ = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandle("ntdll.dll"), "NtQueryInformationProcess");
	NtQueryInformationProcess_(hProc, ProcessBasicInformation, &peb, sizeof(PROCESS_BASIC_INFORMATION), &tmp);
	return (size_t)peb.PebBaseAddress;
}

static unsigned long Major;
static unsigned long Minor;
static unsigned short Build;

void OSVersion(HANDLE handle)
{
	size_t pebAddress = GetProcessPEBAddress(handle);
#ifdef _WIN64
	size_t OSMajorVersionAddress = pebAddress + 0x0118;
	size_t OSMinorVersionAddress = pebAddress + 0x011c;
	size_t OSBuildNumberAddress = pebAddress + 0x0120;
#else
	size_t OSMajorVersionAddress = pebAddress + 0x0a4;
	size_t OSMinorVersionAddress = pebAddress + 0x0a8;
	size_t OSBuildNumberAddress = pebAddress + 0x0ac;
#endif
	ReadProcessMemory(handle, (void*)OSMajorVersionAddress, &Major, sizeof(Major), 0);
	ReadProcessMemory(handle, (void*)OSMinorVersionAddress, &Minor, sizeof(Minor), 0);
	ReadProcessMemory(handle, (void*)OSBuildNumberAddress, &Build, sizeof(Build), 0);
}


bool GetOsVersionInfo(OSVERSIONINFOEX* osvi, SYSTEM_INFO* si)
{
#pragma warning(suppress: 4996)
    if (!GetVersionEx((OSVERSIONINFO*) osvi))
    {
        return false; // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    }

    PGNSI pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");

    if(NULL != pGNSI)
    {
        pGNSI(si);
    }
    else
    {
        GetSystemInfo(si); // Check for unsupported OS
    }



	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	OSVersion(handle);
	osvi->dwMajorVersion = Major;
	osvi->dwMinorVersion = Minor;
	osvi->dwBuildNumber = Build;


    if (VER_PLATFORM_WIN32_NT != osvi->dwPlatformId || osvi->dwMajorVersion <= 4 )
    {
        return false;
    }

	return true;
}

std::string GetOsDescription(const OSVERSIONINFOEX& osvi, const SYSTEM_INFO& si)
{
	std::stringstream os;

    // Test for the specific product.
    if (osvi.dwMajorVersion == 10)
    {
		bool workstation = osvi.wProductType == VER_NT_WORKSTATION;
		switch (osvi.dwMinorVersion)
		{
		case 0:
			os << (workstation ? "Windows 10" : "Windows Server 2016");
			break;
		default:
			os << "Windows [10." << osvi.dwMinorVersion << "] ";
			break;
		}

	    DWORD dwType;
        PGPI pGPI = (PGPI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
        pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
        switch (dwType)
        {
#ifdef PRODUCT_CORE
			case PRODUCT_CORE:
                os << " Home";
                break;
			case PRODUCT_CORE_COUNTRYSPECIFIC:
				os << " Home China";
				break;
			case PRODUCT_CORE_N:
				os << " Home N";
				break;
			case PRODUCT_CORE_SINGLELANGUAGE:
				os << " Home Single Language";
				break;
#endif
				/*
			case PRODUCT_EDUCATION:
				os << " Education";
				break;
			case PRODUCT_EDUCATION_N:
				os << " Education N";
				break;
				*/
			case PRODUCT_ENTERPRISE:
				os << " Enterprise";
				break;
			case PRODUCT_ENTERPRISE_E:
				os << " Enterprise E";
				break;
#ifdef PRODUCT_ENTERPRISE_EVALUATION
			case PRODUCT_ENTERPRISE_EVALUATION:
				os << " Enterprise Evaluation";
				break;
			case PRODUCT_ENTERPRISE_N:
				os << " Enterprise N";
				break;
			case PRODUCT_ENTERPRISE_N_EVALUATION:
				os << " Enterprise N Evaluation";
				break;
#endif
				/*
			case PRODUCT_ENTERPRISE_S:
				os << " Education N";
				break;
			case PRODUCT_ENTERPRISE_S_EVALUATION:
				os << " Education N";
				break;
			case PRODUCT_ENTERPRISE_S_N:
				os << " Education N";
				break;
			case PRODUCT_ENTERPRISE_S_N_EVALUATION:
				os << " Education N";
				break;
				*/
#ifdef PRODUCT_MOBILE_CORE
			case PRODUCT_MOBILE_CORE:
				os << " Mobile";
				break;
#endif
				/*
			case PRODUCT_MOBILE_ENTERPRISE:
				os << " Mobile Enterprise";
				break;
				*/
			case PRODUCT_PROFESSIONAL:
				os << " Pro";
				break;
			case PRODUCT_PROFESSIONAL_N:
				os << " Pro N";
				break;
            default:
                break;
        }
    }
    else if (osvi.dwMajorVersion == 6)
    {
		bool workstation = osvi.wProductType == VER_NT_WORKSTATION;
		switch (osvi.dwMinorVersion)
		{
		case 0:
			os << (workstation ? "Windows Vista" : "Windows Server 2008");
			break;
		case 1:
			os << (workstation ? "Windows 7" : "Windows Server 2008 R2");
			break;
		case 2:
			os << (workstation ? "Windows 8" : "Windows Server 2012");
			break;
		case 3:
			os << (workstation ? "Windows 8.1" : "Windows Server 2012 R2");
			break;
		default:
			os << "Windows [6." << osvi.dwMinorVersion << "] ";
			break;
		}

	    DWORD dwType;
        PGPI pGPI = (PGPI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
        pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
        switch (dwType)
        {
            case PRODUCT_ULTIMATE:
                os << " Ultimate Edition";
                break;
            case PRODUCT_PROFESSIONAL:
                os << " Professional";
                break;
            case PRODUCT_HOME_PREMIUM:
                os << " Home Premium Edition";
                break;
            case PRODUCT_HOME_BASIC:
                os << " Home Basic Edition";
                break;
            case PRODUCT_ENTERPRISE:
                os << " Enterprise Edition";
                break;
            case PRODUCT_BUSINESS:
                os << " Business Edition";
                break;
            case PRODUCT_STARTER:
                os << " Starter Edition";
                break;
            case PRODUCT_CLUSTER_SERVER:
                os << " Cluster Server Edition";
                break;
            case PRODUCT_DATACENTER_SERVER:
                os << " Datacenter Edition";
                break;
            case PRODUCT_DATACENTER_SERVER_CORE:
                os << " Datacenter Edition (core installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER:
                os << " Enterprise Edition";
                break;
            case PRODUCT_ENTERPRISE_SERVER_CORE:
                os << " Enterprise Edition (core installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER_IA64:
                os << " Enterprise Edition for Itanium-based Systems";
                break;
            case PRODUCT_SMALLBUSINESS_SERVER:
                os << " Small Business Server";
                break;
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                os << " Small Business Server Premium Edition";
                break;
            case PRODUCT_STANDARD_SERVER:
                os << " Standard Edition";
                break;
            case PRODUCT_STANDARD_SERVER_CORE:
                os << " Standard Edition (core installation)";
                break;
            case PRODUCT_WEB_SERVER:
                os << " Web Server Edition";
                break;
            default:
                break;
        }
    }
    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
    {
        if( GetSystemMetrics(SM_SERVERR2) )
        {
            os <<  "Windows Server 2003 R2, ";
        }
        else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
        {
            os <<  "Windows Storage Server 2003";
        }
        else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
        {
            os <<  "Windows Home Server";
        }
        else if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
        {
            os <<  "Windows XP Professional x64 Edition";
        }
        else
        {
            os << "Windows Server 2003, ";  // Test for the server type.
        }
        if ( osvi.wProductType != VER_NT_WORKSTATION )
        {
            if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                {
                    os <<  "Datacenter Edition for Itanium-based Systems";
                }
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                {
                    os <<  "Enterprise Edition for Itanium-based Systems";
                }
            }
            else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                {
                    os <<  "Datacenter x64 Edition";
                }
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                {
                    os <<  "Enterprise x64 Edition";
                }
                else
                {
                    os <<  "Standard x64 Edition";
                }
            }
            else
            {
                if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
                {
                    os <<  "Compute Cluster Edition";
                }
                else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                {
                    os <<  "Datacenter Edition";
                }
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                {
                    os <<  "Enterprise Edition";
                }
                else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
                {
                    os <<  "Web Edition";
                }
                else
                {
                    os <<  "Standard Edition";
                }
            }
        }
    }
    else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
    {
        os << "Windows XP ";
        if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
        {
            os <<  "Home Edition";
        }
        else
        {
            os <<  "Professional";
        }
    }
    else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
    {
        os << "Windows 2000 ";
        if ( osvi.wProductType == VER_NT_WORKSTATION )
        {
            os <<  "Professional";
        }
    }
    else
    {
        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
        {
            os <<  "Datacenter Server";
        }
        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
        {
            os <<  "Advanced Server";
        }
        else
        {
            os <<  "Server";
        }
    }

    // Include service pack (if any) and build number.
    if(strlen(osvi.szCSDVersion) > 0)
    {
        os << " " << osvi.szCSDVersion;
    }

	return os.str();
}

void GetCPUInfo(std::string& name, std::string& features, int& cores, int& threads, uint64_t& frequency)
{
#ifndef _M_ARM64
	const char* szFeatures[] =
	{
		"x87 FPU On Chip",
		"Virtual-8086 Mode Enhancement",
		"Debugging Extensions",
		"Page Size Extensions",
		"Time Stamp Counter",
		"RDMSR and WRMSR Support",
		"Physical Address Extensions",
		"Machine Check Exception",
		"CMPXCHG8B Instruction",
		"APIC On Chip",
		"Unknown1",
		"SYSENTER and SYSEXIT",
		"Memory Type Range Registers",
		"PTE Global Bit",
		"Machine Check Architecture",
		"Conditional Move/Compare Instruction",
		"Page Attribute Table",
		"36-bit Page Size Extension",
		"Processor Serial Number",
		"CFLUSH Extension",
		"Unknown2",
		"Debug Store",
		"Thermal Monitor and Clock Ctrl",
		"MMX Technology",
		"FXSAVE/FXRSTOR",
		"SSE Extensions",
		"SSE2 Extensions",
		"Self Snoop",
		"Multithreading Technology",
		"Thermal Monitor",
		"Unknown4",
		"Pending Break Enable"
	};

	char CPUString[0x20];
    char CPUBrandString[0x40];
    int CPUInfo[4] = {-1};
    int nSteppingID = 0;
    int nModel = 0;
    int nFamily = 0;
    int nProcessorType = 0;
    int nExtendedmodel = 0;
    int nExtendedfamily = 0;
    int nBrandIndex = 0;
    int nCLFLUSHcachelinesize = 0;
    int nLogicalProcessors = 0;
    int nAPICPhysicalID = 0;
    int nFeatureInfo = 0;
    int nCacheLineSize = 0;
    int nL2Associativity = 0;
    int nCacheSizeK = 0;
    int nPhysicalAddress = 0;
    int nVirtualAddress = 0;
    int nRet = 0;

    int nCores = 0;
    int nCacheType = 0;
    int nCacheLevel = 0;
    int nMaxThread = 0;
    int nSysLineSize = 0;
    int nPhysicalLinePartitions = 0;
    int nWaysAssociativity = 0;
    int nNumberSets = 0;

    unsigned    nIds, nExIds, i;

    bool    bSSE3Instructions = false;
    bool    bMONITOR_MWAIT = false;
    bool    bCPLQualifiedDebugStore = false;
    bool    bVirtualMachineExtensions = false;
    bool    bEnhancedIntelSpeedStepTechnology = false;
    bool    bThermalMonitor2 = false;
    bool    bSupplementalSSE3 = false;
    bool    bL1ContextID = false;
    bool    bCMPXCHG16B = false;
    bool    bxTPRUpdateControl = false;
    bool    bPerfDebugCapabilityMSR = false;
    bool    bSSE41Extensions = false;
    bool    bSSE42Extensions = false;
    bool    bPOPCNT = false;

    bool    bMultithreading = false;

    bool    bLAHF_SAHFAvailable = false;
    bool    bCmpLegacy = false;
    bool    bSVM = false;
    bool    bExtApicSpace = false;
    bool    bAltMovCr8 = false;
    bool    bLZCNT = false;
    bool    bSSE4A = false;
    bool    bMisalignedSSE = false;
    bool    bPREFETCH = false;
    bool    bSKINITandDEV = false;
    bool    bSYSCALL_SYSRETAvailable = false;
    bool    bExecuteDisableBitAvailable = false;
    bool    bMMXExtensions = false;
    bool    bFFXSR = false;
    bool    b1GBSupport = false;
    bool    bRDTSCP = false;
    bool    b64Available = false;
    bool    b3DNowExt = false;
    bool    b3DNow = false;
    bool    bNestedPaging = false;
    bool    bLBRVisualization = false;
    bool    bFP128 = false;
    bool    bMOVOptimization = false;

    bool    bSelfInit = false;
    bool    bFullyAssociative = false;

    // __cpuid with an InfoType argument of 0 returns the number of
    // valid Ids in CPUInfo[0] and the CPU identification string in
    // the other three array elements. The CPU identification string is
    // not in linear order. The code below arranges the information
    // in a human readable form.
    __cpuid(CPUInfo, 0);
    nIds = CPUInfo[0];
    memset(CPUString, 0, sizeof(CPUString));
    *((int*)CPUString) = CPUInfo[1];
    *((int*)(CPUString+4)) = CPUInfo[3];
    *((int*)(CPUString+8)) = CPUInfo[2];

    // Get the information associated with each valid Id
    for (i=0; i<=nIds; ++i)
    {
        __cpuid(CPUInfo, i);
        //printf_s("\nFor InfoType %d\n", i);
        //printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
        //printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
        //printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
        //printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

        // Interpret CPU feature information.
        if  (i == 1)
        {
            nSteppingID = CPUInfo[0] & 0xf;
            nModel = (CPUInfo[0] >> 4) & 0xf;
            nFamily = (CPUInfo[0] >> 8) & 0xf;
            nProcessorType = (CPUInfo[0] >> 12) & 0x3;
            nExtendedmodel = (CPUInfo[0] >> 16) & 0xf;
            nExtendedfamily = (CPUInfo[0] >> 20) & 0xff;
            nBrandIndex = CPUInfo[1] & 0xff;
            nCLFLUSHcachelinesize = ((CPUInfo[1] >> 8) & 0xff) * 8;
            nLogicalProcessors = ((CPUInfo[1] >> 16) & 0xff);
            nAPICPhysicalID = (CPUInfo[1] >> 24) & 0xff;
            bSSE3Instructions = (CPUInfo[2] & 0x1) || false;
            bMONITOR_MWAIT = (CPUInfo[2] & 0x8) || false;
            bCPLQualifiedDebugStore = (CPUInfo[2] & 0x10) || false;
            bVirtualMachineExtensions = (CPUInfo[2] & 0x20) || false;
            bEnhancedIntelSpeedStepTechnology = (CPUInfo[2] & 0x80) || false;
            bThermalMonitor2 = (CPUInfo[2] & 0x100) || false;
            bSupplementalSSE3 = (CPUInfo[2] & 0x200) || false;
            bL1ContextID = (CPUInfo[2] & 0x300) || false;
            bCMPXCHG16B= (CPUInfo[2] & 0x2000) || false;
            bxTPRUpdateControl = (CPUInfo[2] & 0x4000) || false;
            bPerfDebugCapabilityMSR = (CPUInfo[2] & 0x8000) || false;
            bSSE41Extensions = (CPUInfo[2] & 0x80000) || false;
            bSSE42Extensions = (CPUInfo[2] & 0x100000) || false;
            bPOPCNT= (CPUInfo[2] & 0x800000) || false;
            nFeatureInfo = CPUInfo[3];
            bMultithreading = (nFeatureInfo & (1 << 28)) || false;
        }
    }

    // Calling __cpuid with 0x80000000 as the InfoType argument
    // gets the number of valid extended IDs.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    // Get the information associated with each extended ID.
    for (i=0x80000000; i<=nExIds; ++i)
    {
        __cpuid(CPUInfo, i);
        //printf_s("\nFor InfoType %x\n", i);
        //printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
        //printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
        //printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
        //printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

        if  (i == 0x80000001)
        {
            bLAHF_SAHFAvailable = (CPUInfo[2] & 0x1) || false;
            bCmpLegacy = (CPUInfo[2] & 0x2) || false;
            bSVM = (CPUInfo[2] & 0x4) || false;
            bExtApicSpace = (CPUInfo[2] & 0x8) || false;
            bAltMovCr8 = (CPUInfo[2] & 0x10) || false;
            bLZCNT = (CPUInfo[2] & 0x20) || false;
            bSSE4A = (CPUInfo[2] & 0x40) || false;
            bMisalignedSSE = (CPUInfo[2] & 0x80) || false;
            bPREFETCH = (CPUInfo[2] & 0x100) || false;
            bSKINITandDEV = (CPUInfo[2] & 0x1000) || false;
            bSYSCALL_SYSRETAvailable = (CPUInfo[3] & 0x800) || false;
            bExecuteDisableBitAvailable = (CPUInfo[3] & 0x10000) || false;
            bMMXExtensions = (CPUInfo[3] & 0x40000) || false;
            bFFXSR = (CPUInfo[3] & 0x200000) || false;
            b1GBSupport = (CPUInfo[3] & 0x400000) || false;
            bRDTSCP = (CPUInfo[3] & 0x8000000) || false;
            b64Available = (CPUInfo[3] & 0x20000000) || false;
            b3DNowExt = (CPUInfo[3] & 0x40000000) || false;
            b3DNow = (CPUInfo[3] & 0x80000000) || false;
        }

        // Interpret CPU brand string and cache information.
        if  (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000006)
        {
            nCacheLineSize = CPUInfo[2] & 0xff;
            nL2Associativity = (CPUInfo[2] >> 12) & 0xf;
            nCacheSizeK = (CPUInfo[2] >> 16) & 0xffff;
        }
        else if  (i == 0x80000008)
        {
           nPhysicalAddress = CPUInfo[0] & 0xff;
           nVirtualAddress = (CPUInfo[0] >> 8) & 0xff;
        }
        else if  (i == 0x8000000A)
        {
            bNestedPaging = (CPUInfo[3] & 0x1) || false;
            bLBRVisualization = (CPUInfo[3] & 0x2) || false;
        }
        else if  (i == 0x8000001A)
        {
            bFP128 = (CPUInfo[0] & 0x1) || false;
            bMOVOptimization = (CPUInfo[0] & 0x2) || false;
        }
    }

    // Display all the information in user-friendly format.

    //printf_s("\n\nCPU String: %s\n", CPUString);

	std::stringstream os;
	os.str("");
	os.clear();

    if  (nIds >= 1)
    {
        if  (nFeatureInfo || bSSE3Instructions ||
             bMONITOR_MWAIT || bCPLQualifiedDebugStore ||
             bVirtualMachineExtensions || bEnhancedIntelSpeedStepTechnology ||
             bThermalMonitor2 || bSupplementalSSE3 || bL1ContextID ||
             bCMPXCHG16B || bxTPRUpdateControl || bPerfDebugCapabilityMSR ||
             bSSE41Extensions || bSSE42Extensions || bPOPCNT ||
             bLAHF_SAHFAvailable || bCmpLegacy || bSVM ||
             bExtApicSpace || bAltMovCr8 ||
             bLZCNT || bSSE4A || bMisalignedSSE ||
             bPREFETCH || bSKINITandDEV || bSYSCALL_SYSRETAvailable ||
             bExecuteDisableBitAvailable || bMMXExtensions || bFFXSR || b1GBSupport ||
             bRDTSCP || b64Available || b3DNowExt || b3DNow || bNestedPaging ||
             bLBRVisualization || bFP128 || bMOVOptimization )
        {

            if  (bSSE3Instructions)
                os << "\tSSE3\n";
            if  (bMONITOR_MWAIT)
                os << "\tMONITOR/MWAIT\n";
            if  (bCPLQualifiedDebugStore)
                os << "\tCPL Qualified Debug Store\n";
            if  (bVirtualMachineExtensions)
                os << "\tVirtual Machine Extensions\n";
            if  (bEnhancedIntelSpeedStepTechnology)
                os << "\tEnhanced Intel SpeedStep Technology\n";
            if  (bThermalMonitor2)
                os << "\tThermal Monitor 2\n";
            if  (bSupplementalSSE3)
                os << "\tSupplemental Streaming SIMD Extensions 3\n";
            if  (bL1ContextID)
                os << "\tL1 Context ID\n";
            if  (bCMPXCHG16B)
                os << "\tCMPXCHG16B Instruction\n";
            if  (bxTPRUpdateControl)
                os << "\txTPR Update Control\n";
            if  (bPerfDebugCapabilityMSR)
                os << "\tPerf\\Debug Capability MSR\n";
            if  (bSSE41Extensions)
                os << "\tSSE4.1 Extensions\n";
            if  (bSSE42Extensions)
                os << "\tSSE4.2 Extensions\n";
            if  (bPOPCNT)
                os << "\tPPOPCNT Instruction\n";

            i = 0;
            nIds = 1;
            while (i < (sizeof(szFeatures)/sizeof(const char*)))
            {
                if  (nFeatureInfo & nIds)
                {
                    os << "\t";
                    os << szFeatures[i];
                    os << "\n";
                }

                nIds <<= 1;
                ++i;
            }
            if (bLAHF_SAHFAvailable)
                os << "\tLAHF/SAHF in 64-bit mode\n";
            if (bCmpLegacy)
                os << "\tCore multi-processing legacy mode\n";
            if (bSVM)
                os << "\tSecure Virtual Machine\n";
            if (bExtApicSpace)
                os << "\tExtended APIC Register Space\n";
            if (bAltMovCr8)
                os << "\tAltMovCr8\n";
            if (bLZCNT)
                os << "\tLZCNT instruction\n";
            if (bSSE4A)
                os << "\tSSE4A (EXTRQ, INSERTQ, MOVNTSD, MOVNTSS)\n";
            if (bMisalignedSSE)
                os << "\tMisaligned SSE mode\n";
            if (bPREFETCH)
                os << "\tPREFETCH and PREFETCHW Instructions\n";
            if (bSKINITandDEV)
                os << "\tSKINIT and DEV support\n";
            if (bSYSCALL_SYSRETAvailable)
                os << "\tSYSCALL/SYSRET in 64-bit mode\n";
            if (bExecuteDisableBitAvailable)
                os << "\tExecute Disable Bit\n";
            if (bMMXExtensions)
                os << "\tExtensions to MMX Instructions\n";
            if (bFFXSR)
                os << "\tFFXSR\n";
            if (b1GBSupport)
                os << "\t1GB page support\n";
            if (bRDTSCP)
                os << "\tRDTSCP instruction\n";
            if (b64Available)
                os << "\t64 bit Technology\n";
            if (b3DNowExt)
                os << "\t3Dnow Ext\n";
            if (b3DNow)
                os << "\t3Dnow! instructions\n";
            if (bNestedPaging)
                os << "\tNested Paging\n";
            if (bLBRVisualization)
                os << "\tLBR Visualization\n";
            if (bFP128)
                os << "\tFP128 optimization\n";
            if (bMOVOptimization)
                os << "\tMOVU Optimization\n";
        }
    }

	int nPhysical, nSysThreads, nThreadsPerCore;
	GetProcessorTopology(nPhysical, cores, nSysThreads, threads, nThreadsPerCore);

	name = eraseWhiteSpaces(CPUBrandString);
	features = os.str();
	frequency = GetMeasuredProcessorFreq();
#endif
}

template<typename D, typename F>
std::vector<std::pair<D, F>> histogram(std::vector<D> v)
{
	std::vector<std::pair<D, F>> h;

	for(size_t i=0;i<v.size();i++)
	{
		size_t j=0;
		for(;j<h.size();j++)
			if(h[j].first == v[i])
				break;

		if(j==h.size())
			h.insert(h.begin(),std::pair<D,F>(v[i],1));
		else
			h[j].second += 1;
	}

	return h;
};

struct MemoryBankDesc
{
	std::string	Type;
	std::string Speed;
	UINT64	Capacity;

	bool operator==(const MemoryBankDesc &other) const {
		return this->Type==other.Type && this->Speed==other.Speed && this->Capacity==other.Capacity;
	}

	bool operator!=(const MemoryBankDesc &other) const {
		return !(*this == other);
	}
};

HRESULT GetMemoryInfo(std::vector<MemoryBankDesc>* result) {
	HRESULT hr;
	using std::cout;
	using std::cin;
	using std::endl;

	WMI::WmiQuery memoryQuery(L"ROOT\\CIMV2",L"SELECT Capacity, MemoryType, Speed FROM CIM_PhysicalMemory");
	if (FAILED(hr = memoryQuery.GetError())) {
		return hr;
	}

	MemoryBankDesc memory;
	IWbemClassObject* memoryObj = NULL;
	while (NULL != (memoryObj = memoryQuery.GetNext())) {
		VARIANT vRet;
		if (SUCCEEDED(memoryObj->Get(L"Capacity", 0, &vRet, NULL, NULL))) {
			std::wstring ws(vRet.bstrVal, SysStringLen(vRet.bstrVal));
			memory.Capacity = static_cast<unsigned __int64>(_wtoi64(ws.c_str()));
		}
		UINT32 memoryTypeValue;
		if (SUCCEEDED(memoryObj->Get(L"MemoryType", 0, &vRet, NULL, NULL))) {
			memoryTypeValue = (UINT32)vRet.lVal;
		}
		std::string s;
		switch (memoryTypeValue)
		{
			 case 20: s = "DDR"; break;
			 case 21: s = "DDR2"; break;
			 default:
				   if(memoryTypeValue >= 1 && memoryTypeValue <= 19) s = "non-DDR memory";
				   else s = "DDR3"; //Assume unknown memory(case 0, and 22+) to be DDR3
				   break;
		}
		memory.Type = s;

		UINT64 memorySpeedValue;
		if (SUCCEEDED(memoryObj->Get(L"Speed", 0, &vRet, NULL, NULL))) {
			memorySpeedValue = (UINT32)vRet.lVal;
		}
		stringstream ss;
		ss << memorySpeedValue << " MHz";
		memory.Speed = ss.str();
		result->push_back(memory);
	}

	return S_OK;
}

struct MonitorDesc
{
	std::string	Type;
};

HRESULT GetMonitorsInfo(std::vector<MonitorDesc>* result) {
	HRESULT hr;
	using std::cout;
	using std::cin;
	using std::endl;

	WMI::WmiQuery monitorQuery(L"ROOT\\WMI",L"select * from WmiMonitorID");
	if (FAILED(hr = monitorQuery.GetError())) {
		return hr;
	}
	hr = WBEM_S_FALSE;
	MonitorDesc monitor;
	IWbemClassObject* monitorObj = NULL;
	while (NULL != (monitorObj = monitorQuery.GetNext())) {
		VARIANT vtProp;
		hr = monitorObj->Get(L"UserFriendlyNameLength", 0, &vtProp, 0, 0);
		if(vtProp.lVal == 0) {
			monitor.Type = "Default Display";
			result->push_back(monitor);
			continue;
		}
        // Get the value of the Name property
        hr = monitorObj->Get(L"UserFriendlyName", 0, &vtProp, 0, 0);

		WCHAR* wstr;
		char* str;
		long cnt_elements;

		SAFEARRAY* saValues = vtProp.parray;
		LONG* pVals;
		hr = SafeArrayAccessData(saValues, (void**)&pVals); // direct access to SA memory
		if (SUCCEEDED(hr))
		{
			  long lowerBound, upperBound;  // get array bounds
			  SafeArrayGetLBound(saValues, 1 , &lowerBound);
			  SafeArrayGetUBound(saValues, 1, &upperBound);

			  cnt_elements = upperBound - lowerBound + 1;

			  wstr = new WCHAR[cnt_elements];


			  for (int i = 0; i < cnt_elements; ++i) {
				wstr[i] = (WCHAR)pVals[i];
			  }

			  str = new char[cnt_elements];
			  char dc = ' ';
			  WideCharToMultiByte(CP_ACP,0,wstr,-1,str,(int)cnt_elements,&dc,0);

			  monitor.Type = str;

			  SafeArrayUnaccessData(saValues);

			  delete[] wstr;
			  delete[] str;
		} else
		SafeArrayDestroy(saValues);

		result->push_back(monitor);
		hr = S_OK;
	}

	return hr;
}



struct BatteryDesc
{
	std::string Name;
	std::string	Caption;
	std::string	Description;
	UINT32	BatteryStatus;
	UINT32	Chemistry;
	UINT32	DesignCapacity;
	UINT32	EstimatedChargeRemaining;
	UINT64  DesignVoltage;
};

HRESULT GetBatteryInfo(std::vector<BatteryDesc>* result)
{
	HRESULT hr;
	using std::cout;
	using std::cin;
	using std::endl;

	WMI::WmiQuery batteryQuery(L"ROOT\\CIMV2",L"SELECT Name, BatteryStatus, Caption, Chemistry, Description, DesignCapacity, DesignVoltage, EstimatedChargeRemaining FROM Win32_Battery");
	if (FAILED(hr = batteryQuery.GetError()))
	{
		return hr;
	}

	IWbemClassObject* batteryObj = NULL;
	while (NULL != (batteryObj = batteryQuery.GetNext()))
	{
		BatteryDesc battery;

		VARIANT vRet;
		VariantInit(&vRet);

		bstr_t name;
        if(SUCCEEDED(batteryObj->Get(L"Name", 0, &vRet, NULL, NULL)) && vRet.vt == VT_BSTR)
		{
			name = bstr_t(vRet.bstrVal, true);
			battery.Name = _bstr_t(name);
			VariantClear(&vRet);
		}

		GetString(batteryObj, L"Caption", &battery.Caption);
		GetString(batteryObj, L"Description", &battery.Description);

		if (!GetUInt32(batteryObj, L"BatteryStatus", &battery.BatteryStatus))
			battery.BatteryStatus = 2;	// Unknown

		if (!GetUInt32(batteryObj, L"Chemistry", &battery.Chemistry))
			battery.Chemistry = 2; // Unknown

		if (!GetUInt32(batteryObj, L"DesignCapacity", &battery.DesignCapacity))
			battery.DesignCapacity = 0;

		if (!GetUInt32(batteryObj, L"EstimatedChargeRemaining", &battery.EstimatedChargeRemaining))
			battery.EstimatedChargeRemaining = 0;

		if (!GetUInt64(batteryObj, L"DesignVoltage", &battery.DesignVoltage))
			battery.DesignVoltage = 0;

		if ((name.length() > 0) && (
			battery.BatteryStatus == 2 ||
			battery.Chemistry == 2 ||
			battery.DesignCapacity == -1))
		{
			WMI::WmiQuery portableBatteryQuery(L"ROOT\\CIMV2",L"SELECT Name, BatteryStatus, Chemistry, DesignCapacity, CapacityMultiplier, DesignVoltage FROM Win32_PortableBattery WHERE Name=\"" + name + "\"");

			IWbemClassObject* portableBatteryObj = NULL;
			if (SUCCEEDED(portableBatteryQuery.GetError()) &&
				(NULL != (portableBatteryObj = portableBatteryQuery.GetNext())))
			{
				if (battery.BatteryStatus == 2)
					GetUInt32(portableBatteryObj, L"BatteryStatus", &battery.BatteryStatus);

				if (battery.Chemistry == 2)
					GetUInt32(portableBatteryObj, L"Chemistry", &battery.Chemistry);

				if (battery.DesignCapacity == 0 &&
					GetUInt32(portableBatteryObj, L"DesignCapacity", &battery.DesignCapacity))
				{
					UINT32 capacityMultiplier;
					if (GetUInt32(portableBatteryObj, L"CapacityMultiplier", &capacityMultiplier))
					{
						battery.DesignCapacity *= capacityMultiplier;
					}
				}

				if (battery.DesignVoltage == 0)
					GetUInt64(batteryObj, L"DesignVoltage", &battery.DesignVoltage);
			}
		}

		result->push_back(battery);
	}

	return S_OK;
}


// See more at: http://msdn.microsoft.com/en-us/library/windows/desktop/dd390671%28v=vs.85%29.aspx
void _FreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {
        // pUnk should not be used.
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

struct VideoCaptureDeviceDesc
{
	VideoCaptureDeviceDesc(IMoniker *pMoniker) :
		VideoResX(0),
		VideoResY(0),
		ImageResX(0),
		ImageResY(0)
	{
		IPropertyBag *pPropBag = NULL;
		HRESULT hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);

		if (SUCCEEDED(hr))
		{
			GetString(pPropBag, L"FriendlyName", &FriendlyName);
			GetString(pPropBag, L"Description", &Description);
			GetString(pPropBag, L"DevicePath", &DevicePath);
			GetString(pPropBag, L"CLSID", &CLSID);

			IBaseFilter *pFilter = NULL;
			IEnumPins* pEnumPins = NULL;
			if (SUCCEEDED(pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter)) &&
				SUCCEEDED(pFilter->EnumPins(&pEnumPins)))
			{
				IPin* pPin = NULL;
				while (pEnumPins->Next(1, &pPin, NULL) == S_OK)
				{
					IEnumMediaTypes* pEnumMediaTypes = NULL;
					if (SUCCEEDED(pPin->EnumMediaTypes(&pEnumMediaTypes)))
					{
						AM_MEDIA_TYPE* pMediaType = NULL;
						while (pEnumMediaTypes->Next(1, &pMediaType, NULL) == S_OK)
						{
							if ((pMediaType->formattype == FORMAT_VideoInfo) &&
								(pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
								(pMediaType->pbFormat != NULL))
							{
								VIDEOINFOHEADER* videoInfoHeader = (VIDEOINFOHEADER*)pMediaType->pbFormat;
								VideoResX = max(VideoResX, videoInfoHeader->bmiHeader.biWidth);
								VideoResY = max(VideoResY, videoInfoHeader->bmiHeader.biHeight);
							}
							else if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
								(pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)) &&
								(pMediaType->pbFormat != NULL))
							{
								VIDEOINFOHEADER2* videoInfoHeader2 = (VIDEOINFOHEADER2*)pMediaType->pbFormat;
								VideoResX = max(VideoResX, videoInfoHeader2->bmiHeader.biWidth);
								VideoResY = max(VideoResY, videoInfoHeader2->bmiHeader.biHeight);
							}

						   _FreeMediaType(*pMediaType);
						}
					}

					pPin->Release();
				}
			}

			if (pFilter)
				pFilter->Release();

			if (pEnumPins)
				pEnumPins->Release();

			pPropBag->Release();
		}

		ImageResX = VideoResX;
		ImageResY = VideoResY;
	}

	std::string FriendlyName;
	std::string Description;
	std::string DevicePath;
	std::string CLSID;
	LONG VideoResX;
	LONG VideoResY;
	LONG ImageResX;
	LONG ImageResY;
};

HRESULT GetVideoCaptureDevices(std::vector<VideoCaptureDeviceDesc>* result)
{
	using std::cout;
	using std::cin;
	using std::endl;

	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the video compressor category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK)
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			VideoCaptureDeviceDesc device(pMoniker);
			result->push_back(device);
			pMoniker->Release();
		}

		pEnumCat->Release();
	}

	pSysDevEnum->Release();
	return hr;
}

struct NetworkAdapterDesc
{
	NetworkAdapterDesc(IWbemClassObject* clsObj)
	{
		GetString(clsObj, L"Name", &Name);
		GetString(clsObj, L"Caption", &Caption);
		GetString(clsObj, L"Description", &Description);
		GetString(clsObj, L"AdapterType", &AdapterType);
		GetString(clsObj, L"Manufacturer", &Manufacturer);
		GetString(clsObj, L"NetConnectionID", &NetConnectionID);
		GetString(clsObj, L"PNPDeviceID", &PNPDeviceID);
		GetString(clsObj, L"ProductName", &ProductName);
		GetString(clsObj, L"ServiceName", &ServiceName);
		GetInt32(clsObj, L"AdapterTypeId", &AdapterTypeId);
		GetBool(clsObj, L"PhysicalAdapter", &PhysicalAdapter);
	}

	std::string Name;
	std::string Caption;
	std::string  Description;
	std::string  AdapterType;
	std::string  Manufacturer;
	std::string  NetConnectionID;
	std::string  PNPDeviceID;
	std::string  ProductName;
	std::string  ServiceName;
	INT32  AdapterTypeId;
	bool  PhysicalAdapter;
};

bool IsWirelessAvailable() {
	HRESULT hr;
	WMI::WmiQuery adapterQuery(L"ROOT\\CIMV2",L"select *  from Win32_NetworkAdapter where NetConnectionID like '%Wireless%' or Description like '%Wireless%'");
	if (FAILED(hr = adapterQuery.GetError()))
	{
		return false;
	}

	IWbemClassObject* clsObj = NULL;
	if (NULL != (clsObj = adapterQuery.GetNext()))
		return true;

	return false;
}

bool IsBluetoothAvailable() {
	HRESULT hr;
	WMI::WmiQuery adapterQuery(L"ROOT\\CIMV2",L"select *  from Win32_NetworkAdapter where NetConnectionID like '%Bluetooth%' or Description like '%Bluetooth%'");
	if (FAILED(hr = adapterQuery.GetError()))
	{
		return false;
	}

	IWbemClassObject* clsObj = NULL;
	if (NULL != (clsObj = adapterQuery.GetNext()))
		return true;

	return false;
}

bool IsAccelerometerAvailable() {
	HRESULT hr;
	WMI::WmiQuery adapterQuery(L"ROOT\\CIMV2",L"select * from Win32_PnPEntity where Description like '%Accelerometer%'");
	if (FAILED(hr = adapterQuery.GetError()))
	{
		return false;
	}

	IWbemClassObject* clsObj = NULL;
	if (NULL != (clsObj = adapterQuery.GetNext()))
		return true;

	return false;
}

std::string IsWebcamAvailable() {
	HRESULT hr;
	WMI::WmiQuery adapterQuery(L"ROOT\\CIMV2",L"select Description from Win32_PnPEntity where Description like '%Webcam%' or Description like '%USB Video%'");
	if (FAILED(hr = adapterQuery.GetError()))
	{
		return "";
	}

	IWbemClassObject* clsObj = NULL;
	std::string camDesc;
	if (NULL != (clsObj = adapterQuery.GetNext())) {
		GetString(clsObj, L"Description", &camDesc);
		return camDesc;
	}

	return "";
}

#ifndef _M_ARM64
std::string nvErrorString(NvAPI_Status status)
{
    if (status == NVAPI_NVIDIA_DEVICE_NOT_FOUND) {
        return "NVAPI_NVIDIA_DEVICE_NOT_FOUND";
    } else {
        NvAPI_ShortString szDesc;
        NvAPI_GetErrorMessage(status, szDesc);
        return szDesc;
    }
}
#endif

std::string d3dErrorString(HRESULT hResult)
{
    std::stringstream ss;
    switch (hResult) {
    case D3DERR_DEVICELOST:
        return "D3DERR_DEVICELOST";
    case D3DERR_INVALIDCALL:
        return "D3DERR_INVALIDCALL";
    case D3DERR_NOTAVAILABLE:
        return "D3DERR_NOTAVAILABLE";
    case D3DERR_OUTOFVIDEOMEMORY:
        return "D3DERR_OUTOFVIDEOMEMORY";
        break;
    default:
        ss << "D3DERR_UNDEFINED CODE:" << hResult;
        return ss.str();
    }
}
#ifndef _M_ARM64

MultiGpuInfo collectSli()
{
    MultiGpuInfo multiGpuInfo;
    multiGpuInfo.name = "sli";

    NvAPI_Status status = NvAPI_Initialize();
    if (status != NVAPI_OK) {
        multiGpuInfo.error = nvErrorString(status);
        return multiGpuInfo;
    }

    NV_DISPLAY_DRIVER_VERSION version = { 0 };
    version.version = NV_DISPLAY_DRIVER_VERSION_VER;
#pragma warning(suppress: 4996)
    status = NvAPI_GetDisplayDriverVersion(NVAPI_DEFAULT_HANDLE, &version);
    if (status != NVAPI_OK) {
        multiGpuInfo.error = nvErrorString(status);
        return multiGpuInfo;
    }

    std::stringstream ss;
    ss << version.version;
    multiGpuInfo.driver = ss.str();

    //Dummy D3D device
    LPDIRECT3D9 pD3D = NULL;
    HWND hWnd = FindWindow(NULL, NULL);
    if (!hWnd) {
        multiGpuInfo.driver = "HWND_NULL";
        return multiGpuInfo;
    }

    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) {
        multiGpuInfo.driver = "D3D9_NULL";
        return multiGpuInfo;
    }

    LPDIRECT3DDEVICE9 pDevice = NULL;
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
    //d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
    d3dpp.BackBufferWidth = 1;
    d3dpp.BackBufferHeight = 1;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    HRESULT hResult = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
    if (!SUCCEEDED(hResult)) {
        pD3D->Release();
        multiGpuInfo.error = d3dErrorString(hResult);
        return multiGpuInfo;
    }

    //NvAPI queries
    NV_GET_CURRENT_SLI_STATE sliState;
    sliState.version = NV_GET_CURRENT_SLI_STATE_VER;
    status = NvAPI_D3D_GetCurrentSLIState(pDevice, &sliState);
    if (status != NVAPI_OK) {
        pDevice->Release();
        pD3D->Release();
        multiGpuInfo.error = nvErrorString(status);
        return multiGpuInfo;
    }

    multiGpuInfo.isEnabled = (sliState.maxNumAFRGroups > 1);
    multiGpuInfo.gpuCount = sliState.maxNumAFRGroups;

    pDevice->Release();
    pD3D->Release();
    return multiGpuInfo;
}
#endif

#ifndef _M_ARM64
static const char* agsErrorString(AGSReturnCode rc) {
    switch (rc) {
    case AGS_SUCCESS: return "AGS_SUCCESS";
    case AGS_INVALID_ARGS: return "AGS_INVALID_ARGS";
    case AGS_OUT_OF_MEMORY: return "AGS_OUT_OF_MEMORY";
    case AGS_ERROR_MISSING_DLL: return "AGS_ERROR_MISSING_DLL";
    case AGS_ERROR_LEGACY_DRIVER: return "AGS_ERROR_LEGACY_DRIVER";
    case AGS_EXTENSION_NOT_SUPPORTED: return "AGS_EXTENSION_NOT_SUPPORTED";
    case AGS_ADL_FAILURE: return "AGS_ADL_FAILURE";
    default: return "AGS_UNKNOWN";
    }
}

MultiGpuInfo collectCrossfire()
{
    MultiGpuInfo info;
    info.name = "crossfire";

    AGSContext* ctx = nullptr;
    AGSGPUInfo gpuInfo = {};
    AGSReturnCode rc = agsInit(&ctx, &gpuInfo);
    if (rc != AGS_SUCCESS) {
        info.error = agsErrorString(rc);
        return info;
    }

    if (gpuInfo.driverVersion)
        info.driver = gpuInfo.driverVersion;

    int gpuCount = 0;
    rc = agsGetCrossfireGPUCount(ctx, &gpuCount);
    if (rc != AGS_SUCCESS) {
        info.error = agsErrorString(rc);
        agsDeInit(ctx);
        return info;
    }

    info.isEnabled = gpuCount > 1;
    info.gpuCount = gpuCount;

    agsDeInit(ctx);
    return info;
}
#endif



DeviceInfo WindowsDeviceInfoCollector::collectDeviceInfo()
{
    DeviceDesc device = GetDeviceName();
    DeviceInfo deviceInfo;
    deviceInfo.name = device.Model;
    deviceInfo.manufacturer = device.Manufacturer;
    return deviceInfo;
}



OsInfo WindowsDeviceInfoCollector::collectOsInfo()
{
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX)); osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(SYSTEM_INFO));

    if (!GetOsVersionInfo(&osvi, &si)) {
        return OsInfo();
    }

    std::string name = GetOsDescription(osvi, si);
    int build = (int)osvi.dwBuildNumber;
    std::string arch;
    std::string niceArch;
    if (osvi.dwMajorVersion >= 6) {
        switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            arch = "x64";
            niceArch = "64-bit";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            arch = "x86";
            niceArch = "32-bit";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            arch = "ARM";
            niceArch = "ARM";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            arch = "Intel Itanium";
            niceArch = "IA64";
            break;
        }
    }

    std::ostringstream oss;
    oss << "OS Build " << build;

    OsInfo osInfo;
    osInfo.name = name;
    osInfo.longName = "Microsoft Corporation " + name + " " + niceArch;
    osInfo.shortName = "windows";
    osInfo.arch = arch;
    osInfo.build = oss.str();
    return osInfo;
}



std::vector<DisplayInfo> WindowsDeviceInfoCollector::collectDisplayInfo()
{
    std::vector<MONITORINFOEX> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorInfoEnumProc, (LONG_PTR)&monitors);

    std::stringstream ss;
    std::vector<MonitorDesc> monitornames;
    if (FAILED(GetMonitorsInfo(&monitornames)) || monitornames.size() != monitors.size()) {
        monitornames.clear();
        for (size_t i = 0; i < monitors.size(); i++) {
            MonitorDesc monitor;
            monitor.Type = "Generic PnP Display";
            monitornames.push_back(monitor);
        }
    }

    std::vector<DisplayInfo> displayInfos;
    for (size_t i = 0; i < monitors.size(); i++) {
        RECT& rect = monitors[i].rcMonitor;
        DisplayInfo displayInfo;
        displayInfo.name = monitornames[i].Type;
        displayInfo.widthPixels = abs(rect.right - rect.left);
        displayInfo.heightPixels = abs(rect.bottom - rect.top);
        displayInfos.push_back(displayInfo);
    }
    return displayInfos;
}



std::vector<CpuInfo> WindowsDeviceInfoCollector::collectCpuInfo()
{
    std::vector<CpuInfo> cpuInfos;

    WMI::WmiQuery query(L"ROOT\\CIMV2", L"SELECT * FROM Win32_Processor");
    if (FAILED(query.GetError())) {
        return cpuInfos;
    }

    IWbemClassObject* clsObj = nullptr;
    while ((clsObj = query.GetNext()) != nullptr) {
        CpuInfo cpuInfo;
        cpuInfo.attributes = getFields(clsObj);
        cpuInfo.name = cpuInfo.attributes["Name"];
        cpuInfo.attributes.erase("Name");
        cpuInfo.frequencyMHz = atoi(cpuInfo.attributes["MaxClockSpeed"].c_str());
        cpuInfo.attributes.erase("MaxClockSpeed");
        cpuInfo.cores = atoi(cpuInfo.attributes["NumberOfCores"].c_str());
        cpuInfo.attributes.erase("NumberOfCores");
        cpuInfo.threads = atoi(cpuInfo.attributes["NumberOfLogicalProcessors"].c_str());
        cpuInfo.attributes.erase("NumberOfLogicalProcessors");
        cpuInfos.push_back(cpuInfo);
    }

    return cpuInfos;
}



std::vector<GpuInfo> WindowsDeviceInfoCollector::collectGpuInfo()
{
    std::vector<GpuInfo> gpuInfos;

    WMI::WmiQuery query(L"ROOT\\CIMV2", L"SELECT * FROM CIM_PCVideoController");
    if (FAILED(query.GetError())) {
        return gpuInfos;
    }

    IWbemClassObject* clsObj = nullptr;
    while ((clsObj = query.GetNext()) != nullptr) {
        GpuInfo gpuInfo;
        gpuInfo.attributes = getFields(clsObj);
        gpuInfo.name = gpuInfo.attributes["Description"];
        gpuInfo.attributes.erase("Description");
        gpuInfo.driver = gpuInfo.attributes["DriverVersion"];
        gpuInfo.attributes.erase("DriverVersion");

        std::string gpuPNPDeviceIDStr = gpuInfo.attributes["PNPDeviceID"];
        size_t found = gpuPNPDeviceIDStr.find("VEN_");
        if (found != std::string::npos) {
            gpuInfo.vendor = "0x" + gpuPNPDeviceIDStr.substr(found + 4, 4);
        }
        found = gpuPNPDeviceIDStr.find("DEV_");
        if (found != std::string::npos) {
            gpuInfo.ids = "0x" + gpuPNPDeviceIDStr.substr(found + 4, 4);
        }

        gpuInfos.push_back(gpuInfo);
    }

    return gpuInfos;
}



MultiGpuInfo WindowsDeviceInfoCollector::collectMultiGpuInfo()
{
#ifndef _M_ARM64
    MultiGpuInfo sli = collectSli();
    if (sli.error.empty()) {
        return sli;
    }
    MultiGpuInfo crossfire = collectCrossfire();
    if (crossfire.error.empty()) {
        return crossfire;
    }
#endif
    return MultiGpuInfo();
}



MemoryInfo WindowsDeviceInfoCollector::collectMemoryInfo()
{
    std::vector<MemoryBankDesc> memories;
    GetMemoryInfo(&memories);

    UINT64 size = 0;
    std::for_each(memories.begin(), memories.end(), [&](MemoryBankDesc mb) { size += mb.Capacity; });

    std::vector<std::pair<MemoryBankDesc, int>> collector = histogram<MemoryBankDesc, int>(memories);

    std::stringstream ms;
    for (unsigned int i = 0; i < collector.size(); i++) {
        std::string s = formatSize(collector[i].first.Capacity);
        ms << collector[i].second << " x " << s << ' ' << collector[i].first.Type << " " << collector[i].first.Speed;
        if (i + 1 < collector.size())
            ms << ", ";
    }

    MemoryInfo memoryInfo;
    memoryInfo.details = ms.str();
    memoryInfo.sizeBytes = size;
    return memoryInfo;
}



std::vector<StorageInfo> WindowsDeviceInfoCollector::collectStorageInfo()
{
    std::vector<StorageInfo> storageInfos;

    WMI::WmiQuery query(L"ROOT\\CIMV2", L"SELECT * FROM Win32_DiskDrive");
    if (FAILED(query.GetError())) {
        return storageInfos;
    }

    IWbemClassObject* clsObj = nullptr;
    while ((clsObj = query.GetNext()) != nullptr) {
        StorageInfo storageInfo;
        storageInfo.attributes = getFields(clsObj);
        storageInfo.name = storageInfo.attributes["Caption"];
        storageInfo.attributes.erase("Caption");
        storageInfo.sizeBytes = _atoi64(storageInfo.attributes["Size"].c_str());
        storageInfo.attributes.erase("Size");
        storageInfos.push_back(storageInfo);
    }

    return storageInfos;
}



std::vector<BatteryInfo> WindowsDeviceInfoCollector::collectBatteryInfo()
{
    std::vector<BatteryInfo> batteryInfos;

    std::vector<BatteryDesc> batteries;
    GetBatteryInfo(&batteries);

    SYSTEM_BATTERY_STATE batteryState = {};
    CallNtPowerInformation(SystemBatteryState, NULL, 0, &batteryState, sizeof(batteryState));

    for (size_t i = 0; i < batteries.size(); ++i) {
        BatteryDesc& battery = batteries[i];
        BatteryInfo batteryInfo;

        // See more at: http://msdn.microsoft.com/en-us/library/aa394074%28v=vs.85%29.aspx
        switch (battery.BatteryStatus)
        {
        case 2:	// The system has access to AC so no battery is being discharged. However, the battery is not necessarily charging.
        case 3:	// Fully Charged
            batteryInfo.isConnected = true;
            batteryInfo.isCharging = false;
            break;
        case 1:
        case 4:
        case 5:
            batteryInfo.isConnected = false;
            batteryInfo.isCharging = false;
            break;
        case 6:
        case 7:
        case 8:
        case 9:
            batteryInfo.isConnected = true;
            batteryInfo.isCharging = true;
            break;
        case 10: // Undefined
        case 11:
        default:
            batteryInfo.isConnected = false;
            batteryInfo.isCharging = false;
            break;
        }

        if (batteryState.MaxCapacity > 0) {
            batteryInfo.levelRatio = static_cast<double>(batteryState.RemainingCapacity) /
                    batteryState.MaxCapacity;
        }

        if ((battery.DesignCapacity > 0) && (battery.DesignVoltage > 0)) {
            batteryInfo.capacity_mAh = battery.DesignCapacity * 1000.0 / battery.DesignVoltage;
        }

        switch (battery.Chemistry)
        {
        case 3:
            batteryInfo.technology = "Lead Acid";
            break;
        case 4:
            batteryInfo.technology = "Nickel Cadmium";
            break;
        case 5:
            batteryInfo.technology = "Nickel Metal Hydride";
            break;
        case 6:
            batteryInfo.technology = "Lithium-ion";
            break;
        case 7:
            batteryInfo.technology = "Zinc air";
            break;
        case 8:
            batteryInfo.technology = "Lithium Polymer";
            break;
        default:
            batteryInfo.technology = "Unknown";
            break;
        }

        batteryInfos.push_back(batteryInfo);
    }

    return batteryInfos;
}



std::vector<CameraInfo> WindowsDeviceInfoCollector::collectCameraInfo()
{
    std::vector<CameraInfo> cameraInfos;

    std::string wc = IsWebcamAvailable();
    if (wc != "") {
        CameraInfo cameraInfo;
        cameraInfo.name = wc;
        cameraInfo.type = "CAMERA_TYPE_FRONT";
        cameraInfos.push_back(cameraInfo);
    }

    HRESULT hr;
    ICreateDevEnum *pSysDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
    if (FAILED(hr))
    {
        return cameraInfos;
    }

    // Obtain a class enumerator for the video compressor category.
    IEnumMoniker *pEnumCat = NULL;
    hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

    if (hr == S_OK)
    {
        // Enumerate the monikers.
        IMoniker *pMoniker = NULL;
        ULONG cFetched;
        while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
        {
            VideoCaptureDeviceDesc device(pMoniker);
            CameraInfo cameraInfo;
            cameraInfo.name = device.FriendlyName;
            cameraInfo.pictureWidthPixels = device.ImageResX;
            cameraInfo.pictureHeightPixels = device.ImageResY;
            cameraInfo.pictureResolutionMP = device.ImageResX * device.ImageResY * 0.000001;
            cameraInfo.videoWidthPixels = device.VideoResX;
            cameraInfo.videoHeightPixels = device.VideoResY;
            cameraInfo.videoResolutionMP = device.VideoResX * device.VideoResY * 0.000001;
            cameraInfos.push_back(cameraInfo);
            pMoniker->Release();
        }

        pEnumCat->Release();
    }
    pSysDevEnum->Release();

    return cameraInfos;
}



FeatureInfo WindowsDeviceInfoCollector::collectFeatureInfo()
{
    FeatureInfo featureInfo;
    featureInfo.features["wifi"] = IsWirelessAvailable();
    featureInfo.features["bluetooth"] = IsBluetoothAvailable();
    featureInfo.features["accelerometer"] = IsAccelerometerAvailable();
    featureInfo.features["camera (face)"] = !IsWebcamAvailable().empty();
    return featureInfo;
}



SensorInfo WindowsDeviceInfoCollector::collectSensorInfo()
{
    return SensorInfo();
}
