/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "DXFeatureTask.h"

//--- 3D API-----------
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Platform;
using namespace Platform::Collections;
//---------------------

using namespace concurrency;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
using namespace WindowsPhoneSystemInfo;
#else
using namespace WindowsRTSystemInfo;
#endif


DXFeatureTask::DXFeatureTask()
{
}

DXInfo::DXInfo(DXInfo^ copy)
{
	this->m_d3dDescription = copy->m_d3dDescription;
	this->m_d3dFeatureLevel = copy->m_d3dFeatureLevel;
	this->m_DedicatedSystemMemory = copy->m_DedicatedSystemMemory;
	this->m_DedicatedVideoMemory = copy->m_DedicatedVideoMemory;
	this->m_deviceID = copy->m_deviceID;
	this->m_isDefault = copy->m_isDefault;
	this->m_MSAA = copy->m_MSAA;
	this->m_revision = copy->m_revision;
	this->m_SharedSystemMemory = copy->m_SharedSystemMemory;
	this->m_subSysID = copy->m_subSysID;
}

String^ ConvertEnumToString(D3D_FEATURE_LEVEL d3dFeatureLevel)
{
	String^ featureLevelStr = ref new String();

	switch (d3dFeatureLevel)
	{
	case D3D_FEATURE_LEVEL_9_1:
		featureLevelStr = "D3D_FEATURE_LEVEL_9_1";
		break;
	case D3D_FEATURE_LEVEL_9_2:
		featureLevelStr = "D3D_FEATURE_LEVEL_9_2";
		break;
	case D3D_FEATURE_LEVEL_9_3:
		featureLevelStr = "D3D_FEATURE_LEVEL_9_3";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		featureLevelStr = "D3D_FEATURE_LEVEL_10_0";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		featureLevelStr = "D3D_FEATURE_LEVEL_10_1";
		break;
	case D3D_FEATURE_LEVEL_11_0:
		featureLevelStr = "D3D_FEATURE_LEVEL_11_0";
		break;
	case D3D_FEATURE_LEVEL_11_1:
		featureLevelStr = "D3D_FEATURE_LEVEL_11_1";
		break;
	default:
		featureLevelStr = "none";
	}

	return featureLevelStr;
}

//D3D FeatureLevel
IAsyncOperation<bool>^ DXFeatureTask::CollectDXInfo()
{
	return create_async([this]() -> bool
	{
		D3D_FEATURE_LEVEL d3dFeatureLevel;
		d3dFeatureLevel = D3D_FEATURE_LEVEL_9_1;
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		// Create the Direct3D 11 API device object and a corresponding context.
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;

		IDXGIFactory1 *dxgiFactory;
		if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&dxgiFactory)))
		{
			return false;
			//log_error << "Couldn't create DXGIFactory" << endl;
		}

		m_devices = ref new Vector<DXInfo^>();

		IDXGIAdapter *dxgiAdapter;
		UINT i = 0;
		while (dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXInfo^ gpu = ref new DXInfo();

			auto& myAdapter = *dxgiAdapter;
			auto adapterDescription = new DXGI_ADAPTER_DESC();
			myAdapter.GetDesc(adapterDescription);

			if ((adapterDescription->VendorId == 0x1414) && (adapterDescription->DeviceId == 0x8c))
			{
				i++;
				continue; // http://msdn.microsoft.com/en-us/library/windows/desktop/bb205075(v=vs.85).aspx#WARP_new_for_Win8
			}

			gpu->m_d3dDescription = ref new String(adapterDescription->Description);
			gpu->m_DedicatedSystemMemory = (adapterDescription->DedicatedSystemMemory / 1024 / 1024).ToString();
			gpu->m_DedicatedVideoMemory = (adapterDescription->DedicatedVideoMemory / 1024 / 1024).ToString();
			gpu->m_SharedSystemMemory = (adapterDescription->SharedSystemMemory / 1024 / 1024).ToString();

			gpu->m_vendorID = adapterDescription->VendorId;
			gpu->m_deviceID = adapterDescription->DeviceId;
			gpu->m_subSysID = adapterDescription->SubSysId;
			gpu->m_revision = adapterDescription->Revision;

			HRESULT hr = D3D11CreateDevice(
				dxgiAdapter,					// Specify nullptr to use the default adapter.
				D3D_DRIVER_TYPE_UNKNOWN,	// Create a device using the hardware graphics driver.
				0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
				creationFlags,				// Set debug and Direct2D compatibility flags.
				featureLevels,				// List of feature levels this app can support.
				ARRAYSIZE(featureLevels),	// Size of the list above.
				D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Windows Store apps.
				&device,					// Returns the Direct3D device created.
				&d3dFeatureLevel,			// Returns feature level of device created.
				&context					// Returns the device immediate context.
				);

			if (FAILED(hr))
			{
				// If the initialization fails, fall back to the WARP device.
				// For more information on WARP, see: 
				// http://go.microsoft.com/fwlink/?LinkId=286690
				ThrowIfFailed(
					D3D11CreateDevice(
					dxgiAdapter,
					D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
					0,
					creationFlags,
					featureLevels,
					ARRAYSIZE(featureLevels),
					D3D11_SDK_VERSION,
					&device,
					&d3dFeatureLevel,
					&context
					)
					);
			}

			UINT qualityLevels;
			device.Get()->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &qualityLevels);

			/*
			D3D11_FEATURE_DATA_ARCHITECTURE_INFO arch_info;
			device.Get()->CheckFeatureSupport(D3D11_FEATURE_ARCHITECTURE_INFO, &arch_info, sizeof(arch_info));
			*/

			gpu->m_MSAA = qualityLevels;

			gpu->m_d3dFeatureLevel = ConvertEnumToString(d3dFeatureLevel);

			if (i == 0)
				gpu->m_isDefault = true;
			else
				gpu->m_isDefault = false;

			m_devices->Append(gpu);
			i++;

			dxgiAdapter->Release();
		}
		dxgiFactory->Release();

		return true;
	});
}

