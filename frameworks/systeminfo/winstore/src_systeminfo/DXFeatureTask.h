/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include <d3d11.h>
#include <d3d11_1.h>
#include <winerror.h>	// For FAILED
#include <ppltasks.h>	// For create_task
#include <collection.h>

namespace WFC = Windows::Foundation::Collections;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
	namespace WindowsPhoneSystemInfo
#else
	namespace WindowsRTSystemInfo
#endif


{
	public ref class DXInfo sealed
	{
	public:
		DXInfo()
		{

		}

		DXInfo(DXInfo^ copy);

		property Platform::String^ FeatureLevel
		{
			Platform::String^ get() { return m_d3dFeatureLevel; }
		}

		property Platform::String^ Description
		{
			Platform::String^ get() { return m_d3dDescription; }
			void set(Platform::String^ value){ m_d3dDescription = value; }
		}

		property Platform::String^ DedicatedSystemMemory
		{
			Platform::String^ get() { return m_DedicatedSystemMemory; }
		}

		property Platform::String^ DedicatedVideoMemory
		{
			Platform::String^ get() { return m_DedicatedVideoMemory; }
		}

		property Platform::String^ SharedSystemMemory
		{
			Platform::String^ get() { return m_SharedSystemMemory; }
		}

		property UINT VendorID
		{
			UINT get() { return m_vendorID; }
			void set(UINT value) { m_vendorID = value; }
		}

		property UINT DeviceID
		{
			UINT get() { return m_deviceID; }
			void set(UINT value) { m_deviceID = value; }
		}

		property UINT SubSysID
		{
			UINT get() { return m_subSysID; }
		}

		property UINT Revision
		{
			UINT get() { return m_revision; }
		}

		property UINT MSAA
		{
			UINT get() { return m_MSAA; }
		}

		property bool IsDefault
		{
			bool get() { return m_isDefault; }
			void set(bool value) { m_isDefault = value; }
		}

	internal:
		Platform::String^ m_d3dFeatureLevel;
		Platform::String^ m_d3dDescription;
		Platform::String^ m_DedicatedSystemMemory;
		Platform::String^ m_DedicatedVideoMemory;
		Platform::String^ m_SharedSystemMemory;

		UINT m_vendorID;
		UINT m_deviceID;
		UINT m_subSysID;
		UINT m_revision;
		UINT m_MSAA;

		bool m_isDefault;
	};

	public ref class DXFeatureTask sealed
    {
    public:
		DXFeatureTask();
	
		property WFC::IVector<DXInfo^>^ GPUs
		{
			WFC::IVector<DXInfo^>^ get() { return m_devices; }
		}

		Windows::Foundation::IAsyncOperation<bool>^ CollectDXInfo();

	private:
		Platform::Collections::Vector<DXInfo^>^ m_devices;
	};

	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}
}