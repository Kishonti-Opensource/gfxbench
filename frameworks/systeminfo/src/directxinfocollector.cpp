/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "directxinfocollector.h"
#include "dxgi1_2.h"
#include "ngrtl/core/ng/log.h"

#define SYSTEMINFO_WIN10_AVAILABLE 1
#define SYSTEMINFO_WIN8_AVAILABLE 1

#ifdef SYSTEMINFO_WIN10_AVAILABLE
	// Windows SDK 10.0.14393 or above required
	#include <d3d12.h>
	#include <d3d11_4.h>
#else
	#include <d3d11.h>
#endif

#include <codecvt>
#include <cassert>


#define DX11_QUERY_FEATURE(device, feature) SUCCEEDED(device->CheckFeatureSupport(D3D11_FEATURE_ ## feature, &feature, sizeof(feature)))
#define DX12_QUERY_FEATURE(device, feature) SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ ## feature, &feature, sizeof(feature)))
#define ADD_FEATURE(device_info, data, field_name) AddFeature(device_info, #field_name, data.field_name)
#define CASE_ADD_FEATURE(basename, c) case basename ## _ ## c:  AddFeature(device_info, name, #c); break

static std::string ws2s(const std::wstring& wstr)
{
	std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

// Note: returns 0.0 if not between 9.1 and 12.1
static void ExtractFeatureLevelVersion(D3D_FEATURE_LEVEL featureLevel, int &major, int &minor)
{
	major = 0;
	minor = 0;

	switch (featureLevel)
	{
#ifdef SYSTEMINFO_WIN10_AVAILABLE
		// since Windows 10 SDK
	case D3D_FEATURE_LEVEL_12_1: major = 12; minor = 1; break;
	case D3D_FEATURE_LEVEL_12_0: major = 12; minor = 0; break;
#endif
#ifdef SYSTEMINFO_WIN8_AVAILABLE
		// since Windows 8.1 SDK
	case D3D_FEATURE_LEVEL_11_1: major = 11; minor = 1; break;
#endif()
	case D3D_FEATURE_LEVEL_11_0: major = 11; minor = 0; break;
	case D3D_FEATURE_LEVEL_10_1: major = 10; minor = 1; break;
	case D3D_FEATURE_LEVEL_10_0: major = 10; minor = 0; break;
	case D3D_FEATURE_LEVEL_9_3: major = 9; minor = 3; break;
	case D3D_FEATURE_LEVEL_9_2: major = 9; minor = 2; break;
	case D3D_FEATURE_LEVEL_9_1: major = 9; minor = 1; break;
	}
}

static inline void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, const char *value)
{
	device_info.features_string.push_back(std::pair<std::string, std::string>(name, value));
}

static inline void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, BOOL value)
{
	device_info.features_bool.push_back(std::pair<std::string, bool>(name, value == TRUE));
}

static inline void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, UINT value)
{
	device_info.features_uint32.push_back(std::pair<std::string, uint32_t>(name, static_cast<uint32_t>(value)));
}

#ifdef SYSTEMINFO_WIN10_AVAILABLE
static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D12_SHADER_MIN_PRECISION_SUPPORT value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D12_SHADER_MIN_PRECISION_SUPPORT, NONE);
		CASE_ADD_FEATURE(D3D12_SHADER_MIN_PRECISION_SUPPORT, 10_BIT);
		CASE_ADD_FEATURE(D3D12_SHADER_MIN_PRECISION_SUPPORT, 16_BIT);
	}
}

static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D12_TILED_RESOURCES_TIER value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D12_TILED_RESOURCES_TIER, NOT_SUPPORTED);
		CASE_ADD_FEATURE(D3D12_TILED_RESOURCES_TIER, 1);
		CASE_ADD_FEATURE(D3D12_TILED_RESOURCES_TIER, 2);
		CASE_ADD_FEATURE(D3D12_TILED_RESOURCES_TIER, 3);
	}
}

static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D12_RESOURCE_BINDING_TIER value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D12_RESOURCE_BINDING_TIER, 1);
		CASE_ADD_FEATURE(D3D12_RESOURCE_BINDING_TIER, 2);
		CASE_ADD_FEATURE(D3D12_RESOURCE_BINDING_TIER, 3);
	}
}

static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D12_CONSERVATIVE_RASTERIZATION_TIER value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D12_CONSERVATIVE_RASTERIZATION_TIER, NOT_SUPPORTED);
		CASE_ADD_FEATURE(D3D12_CONSERVATIVE_RASTERIZATION_TIER, 1);
		CASE_ADD_FEATURE(D3D12_CONSERVATIVE_RASTERIZATION_TIER, 2);
		CASE_ADD_FEATURE(D3D12_CONSERVATIVE_RASTERIZATION_TIER, 3);
	}
}

static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D12_CROSS_NODE_SHARING_TIER value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D12_CROSS_NODE_SHARING_TIER, NOT_SUPPORTED);
		CASE_ADD_FEATURE(D3D12_CROSS_NODE_SHARING_TIER, 1_EMULATED);
		CASE_ADD_FEATURE(D3D12_CROSS_NODE_SHARING_TIER, 1);
		CASE_ADD_FEATURE(D3D12_CROSS_NODE_SHARING_TIER, 2);
	}
}

static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D12_RESOURCE_HEAP_TIER value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D12_RESOURCE_HEAP_TIER, 1);
		CASE_ADD_FEATURE(D3D12_RESOURCE_HEAP_TIER, 2);
	}
}

static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D_SHADER_MODEL value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D_SHADER_MODEL, 5_1);
		CASE_ADD_FEATURE(D3D_SHADER_MODEL, 6_0);
	}
}

static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D_ROOT_SIGNATURE_VERSION value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D_ROOT_SIGNATURE_VERSION, 1_0);
		CASE_ADD_FEATURE(D3D_ROOT_SIGNATURE_VERSION, 1_1);
	}
}

static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D11_CONSERVATIVE_RASTERIZATION_TIER value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D11_CONSERVATIVE_RASTERIZATION, NOT_SUPPORTED);
		CASE_ADD_FEATURE(D3D11_CONSERVATIVE_RASTERIZATION, TIER_1);
		CASE_ADD_FEATURE(D3D11_CONSERVATIVE_RASTERIZATION, TIER_2);
		CASE_ADD_FEATURE(D3D11_CONSERVATIVE_RASTERIZATION, TIER_3);
	}
}

#endif

#ifdef SYSTEMINFO_WIN8_AVAILABLE
static void AddFeature(sysinf::DirectxDeviceInfo &device_info, const char *name, D3D11_TILED_RESOURCES_TIER value)
{
	switch (value)
	{
		CASE_ADD_FEATURE(D3D11_TILED_RESOURCES, NOT_SUPPORTED);
		CASE_ADD_FEATURE(D3D11_TILED_RESOURCES, TIER_1);
		CASE_ADD_FEATURE(D3D11_TILED_RESOURCES, TIER_2);
		CASE_ADD_FEATURE(D3D11_TILED_RESOURCES, TIER_3);
	}
}
#endif


void collectDX12Info(IDXGIAdapter1* pAdapter, const DXGI_ADAPTER_DESC1& AdapterDesc, sysinf::SystemInfo& systemInfo);
void collectDX11Info(IDXGIAdapter1* pAdapter, const DXGI_ADAPTER_DESC1& AdapterDesc, sysinf::SystemInfo& systemInfo);

void sysinf::collectDirectxInfo(SystemInfo& systemInfo)
{
	IDXGIAdapter1 *pAdapter = nullptr;
	IDXGIFactory2 *pFactory = nullptr;

	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&pFactory)))
	{
		return ;
	}

	for (UINT i = 0; /*empty*/ ; ++i)
	{
		if (pAdapter != nullptr)
		{
			pAdapter->Release();
			pAdapter = nullptr;
		}

		if (pFactory->EnumAdapters1(i, &pAdapter) != S_OK)
		{
			break;
		}

		DXGI_ADAPTER_DESC1 AdapterDesc = {};
		if (FAILED(pAdapter->GetDesc1(&AdapterDesc)))
		{
			continue;
		}

		//skip Microsoft software adapter (WARP)
		if ((AdapterDesc.Flags & 2) > 0 // DXGI_ADAPTER_FLAG_SOFTWARE
			|| (AdapterDesc.VendorId == 5140 && AdapterDesc.DeviceId == 140)) // VendorId of 0x1414 and a DeviceID of 0x8c
		{
			continue;
		}

#if 0
		// This filter skips adapters where there's no display attached
		IDXGIOutput *adapterOutput = nullptr;
		if (pAdapter->EnumOutputs(0, &adapterOutput) == DXGI_ERROR_NOT_FOUND)
		{
			continue;
		}
		else if (adapterOutput != nullptr)
		{
			adapterOutput->Release();
		}
#endif

		collectDX11Info(pAdapter, AdapterDesc, systemInfo);
		collectDX12Info(pAdapter, AdapterDesc, systemInfo);
	}

	if (pAdapter != nullptr)
	{
		pAdapter->Release();
		pAdapter = nullptr;
	}

	if (pFactory != nullptr)
	{
		pFactory->Release();
		pFactory = nullptr;
	}
}


void collectDX12Info(IDXGIAdapter1* pAdapter, const DXGI_ADAPTER_DESC1& AdapterDesc, sysinf::SystemInfo& systemInfo)
{
#ifdef SYSTEMINFO_WIN10_AVAILABLE
	HMODULE D3D12Module = LoadLibrary(TEXT("d3d12.dll"));
	if (D3D12Module)
	{
		PFN_D3D12_CREATE_DEVICE D3D12CreateDevice_ = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(D3D12Module, "D3D12CreateDevice");
		if (D3D12CreateDevice_)
		{
			ID3D12Device* dx12_device = nullptr;
			if (SUCCEEDED(D3D12CreateDevice_(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dx12_device))))
			{
				sysinf::DirectxDeviceInfo device_info;

				{
					D3D12_FEATURE_DATA_D3D12_OPTIONS D3D12_OPTIONS{};
					D3D12_FEATURE_DATA_ARCHITECTURE ARCHITECTURE{};
					D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT GPU_VIRTUAL_ADDRESS_SUPPORT{};
					D3D12_FEATURE_DATA_SHADER_MODEL SHADER_MODEL{};
					D3D12_FEATURE_DATA_D3D12_OPTIONS1 D3D12_OPTIONS1{};
					D3D12_FEATURE_DATA_ROOT_SIGNATURE ROOT_SIGNATURE{};

					if (DX12_QUERY_FEATURE(dx12_device, D3D12_OPTIONS))
					{
						ADD_FEATURE(device_info, D3D12_OPTIONS, DoublePrecisionFloatShaderOps);
						ADD_FEATURE(device_info, D3D12_OPTIONS, OutputMergerLogicOp);
						ADD_FEATURE(device_info, D3D12_OPTIONS, PSSpecifiedStencilRefSupported);
						ADD_FEATURE(device_info, D3D12_OPTIONS, MinPrecisionSupport);
						ADD_FEATURE(device_info, D3D12_OPTIONS, TiledResourcesTier);
						ADD_FEATURE(device_info, D3D12_OPTIONS, ResourceBindingTier);
						ADD_FEATURE(device_info, D3D12_OPTIONS, TypedUAVLoadAdditionalFormats);
						ADD_FEATURE(device_info, D3D12_OPTIONS, ROVsSupported);
						ADD_FEATURE(device_info, D3D12_OPTIONS, ConservativeRasterizationTier);
						ADD_FEATURE(device_info, D3D12_OPTIONS, MaxGPUVirtualAddressBitsPerResource);
						ADD_FEATURE(device_info, D3D12_OPTIONS, StandardSwizzle64KBSupported);
						ADD_FEATURE(device_info, D3D12_OPTIONS, CrossNodeSharingTier);
						ADD_FEATURE(device_info, D3D12_OPTIONS, CrossAdapterRowMajorTextureSupported);
						ADD_FEATURE(device_info, D3D12_OPTIONS, StandardSwizzle64KBSupported);
						ADD_FEATURE(device_info, D3D12_OPTIONS, VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation);
						ADD_FEATURE(device_info, D3D12_OPTIONS, ResourceHeapTier);
					}

					if (DX12_QUERY_FEATURE(dx12_device, ARCHITECTURE))
					{
						// NodeIndex is skipped
						ADD_FEATURE(device_info, ARCHITECTURE, TileBasedRenderer);
						ADD_FEATURE(device_info, ARCHITECTURE, UMA);
						ADD_FEATURE(device_info, ARCHITECTURE, CacheCoherentUMA);
					}

					if (DX12_QUERY_FEATURE(dx12_device, GPU_VIRTUAL_ADDRESS_SUPPORT))
					{
						ADD_FEATURE(device_info, GPU_VIRTUAL_ADDRESS_SUPPORT, MaxGPUVirtualAddressBitsPerResource);
						ADD_FEATURE(device_info, GPU_VIRTUAL_ADDRESS_SUPPORT, MaxGPUVirtualAddressBitsPerProcess);
					}

					if (DX12_QUERY_FEATURE(dx12_device, SHADER_MODEL))
					{
						ADD_FEATURE(device_info, SHADER_MODEL, HighestShaderModel);
					}

					if (DX12_QUERY_FEATURE(dx12_device, D3D12_OPTIONS1))
					{
						ADD_FEATURE(device_info, D3D12_OPTIONS1, WaveOps);
						ADD_FEATURE(device_info, D3D12_OPTIONS1, WaveLaneCountMin);
						ADD_FEATURE(device_info, D3D12_OPTIONS1, WaveLaneCountMax);
						ADD_FEATURE(device_info, D3D12_OPTIONS1, TotalLaneCount);
						ADD_FEATURE(device_info, D3D12_OPTIONS1, ExpandedComputeResourceStates);
						ADD_FEATURE(device_info, D3D12_OPTIONS1, Int64ShaderOps);
					}

					if (DX12_QUERY_FEATURE(dx12_device, ROOT_SIGNATURE))
					{
						ADD_FEATURE(device_info, ROOT_SIGNATURE, HighestVersion);
					}

					// D3D12_FEATURE_FEATURE_LEVELS are queried below separately

					// D3D12_FEATURE_DATA_FORMAT_SUPPORT is not queried because it is a ~118x45 matrix
					// and most of it is not useful for systeminfo

					// Skipped because it doesn't change with the hardware:
					// D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS - Describes the image quality levels for a given format and sample count.
					// D3D12_FEATURE_FORMAT_INFO - Describes the DXGI data format. Returns the number of planes stored by the format.
				}


				// Get highest supported feature level
				D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
				{
					const D3D_FEATURE_LEVEL featureLevels[] =
					{
						D3D_FEATURE_LEVEL_12_1,
						D3D_FEATURE_LEVEL_12_0,
						D3D_FEATURE_LEVEL_11_1,
						D3D_FEATURE_LEVEL_11_0,
					};

					D3D12_FEATURE_DATA_FEATURE_LEVELS query =
					{
						_countof(featureLevels), featureLevels, D3D_FEATURE_LEVEL_11_0
					};

					if (SUCCEEDED(dx12_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &query, sizeof(query))))
					{
						featureLevel = query.MaxSupportedFeatureLevel;
					}
				}

				device_info.name = ws2s(AdapterDesc.Description);
				device_info.vendorID = AdapterDesc.VendorId;
				device_info.deviceID = AdapterDesc.DeviceId;
				ExtractFeatureLevelVersion(featureLevel, device_info.majorVersion, device_info.minorVersion);
				dx12_device->Release();

				assert(sizeof(device_info.luid) == sizeof(AdapterDesc.AdapterLuid));
				memcpy(&device_info.luid, &AdapterDesc.AdapterLuid, sizeof(device_info.luid));

				systemInfo.directx12Info.dx12_devices.push_back(device_info);
				systemInfo.hasDirectx12 = true;
			}
		}

		FreeModule(D3D12Module);
	}
#endif
}


void collectDX11Info(IDXGIAdapter1* pAdapter, const DXGI_ADAPTER_DESC1& AdapterDesc, sysinf::SystemInfo& systemInfo)
{
	//skip querying D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	//because these flags doesn't works on Windows 7
	D3D_FEATURE_LEVEL featureLevels[] =
	{
//#ifdef SYSTEMINFO_WIN10_AVAILABLE
//		// since Windows 10 SDK
//		D3D_FEATURE_LEVEL_12_1,
//		D3D_FEATURE_LEVEL_12_0,
//#endif
#ifdef SYSTEMINFO_WIN8_AVAILABLE
		// since Windows 8.1 SDK
		D3D_FEATURE_LEVEL_11_1,
#endif
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_1;
	UINT creationFlags = 0;
	D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_HARDWARE;

	// Create the DX11 API device object, and get a corresponding context.
	ID3D11Device* dx11_device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	HRESULT hr = D3D11CreateDevice(
		NULL,                    // specify null to use the default adapter
		m_driverType,
		NULL,                    // leave as nullptr unless software device
		creationFlags,              // optionally set debug and Direct2D compatibility flags
		featureLevels,              // list of feature levels this app can support
		ARRAYSIZE(featureLevels),   // number of entries in above list
		D3D11_SDK_VERSION,          // always set this to D3D11_SDK_VERSION
		&dx11_device,                    // returns the Direct3D device created
		&featureLevel,            // returns feature level of device created
		&context                    // returns the device immediate context
	);

	if (hr == S_OK)
	{
		sysinf::DirectxDeviceInfo device_info;

		{
			D3D11_FEATURE_DATA_THREADING THREADING{};
			D3D11_FEATURE_DATA_DOUBLES DOUBLES{};
			D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS D3D10_X_HARDWARE_OPTIONS{};

#ifdef SYSTEMINFO_WIN8_AVAILABLE
			// Requires Direct3D 11.1 (since Windows 8 SDK)
			D3D11_FEATURE_DATA_D3D11_OPTIONS D3D11_OPTIONS{};
			D3D11_FEATURE_DATA_ARCHITECTURE_INFO ARCHITECTURE_INFO{};
			D3D11_FEATURE_DATA_D3D9_OPTIONS D3D9_OPTIONS{};
			D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT SHADER_MIN_PRECISION_SUPPORT{};
			// since Windows 8.1 SDK
			D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT D3D9_SHADOW_SUPPORT{};
			D3D11_FEATURE_DATA_D3D11_OPTIONS1 D3D11_OPTIONS1{};
			D3D11_FEATURE_DATA_D3D9_SIMPLE_INSTANCING_SUPPORT D3D9_SIMPLE_INSTANCING_SUPPORT{};
			D3D11_FEATURE_DATA_MARKER_SUPPORT MARKER_SUPPORT{};
			D3D11_FEATURE_DATA_D3D9_OPTIONS1 D3D9_OPTIONS1{};
#endif
#ifdef SYSTEMINFO_WIN10_AVAILABLE
			// since Windows 10 SDK
			D3D11_FEATURE_DATA_D3D11_OPTIONS2 D3D11_OPTIONS2{};
			D3D11_FEATURE_DATA_D3D11_OPTIONS3 D3D11_OPTIONS3{};
			// since Windows 10 SDK 10.0.10240
			D3D11_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT GPU_VIRTUAL_ADDRESS_SUPPORT{};
			// since Windows 10 SDK 10.0.14393
			D3D11_FEATURE_DATA_D3D11_OPTIONS4 D3D11_OPTIONS4{};
#endif

			if (DX11_QUERY_FEATURE(dx11_device, THREADING))
			{
				ADD_FEATURE(device_info, THREADING, DriverConcurrentCreates);
				ADD_FEATURE(device_info, THREADING, DriverCommandLists);
			}

			if (DX11_QUERY_FEATURE(dx11_device, DOUBLES))
			{
				ADD_FEATURE(device_info, DOUBLES, DoublePrecisionFloatShaderOps);
			}

			if (DX11_QUERY_FEATURE(dx11_device, D3D10_X_HARDWARE_OPTIONS))
			{
				ADD_FEATURE(device_info, D3D10_X_HARDWARE_OPTIONS, ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x);
			}

#ifdef SYSTEMINFO_WIN8_AVAILABLE
			if (DX11_QUERY_FEATURE(dx11_device, D3D11_OPTIONS))
			{
				ADD_FEATURE(device_info, D3D11_OPTIONS, OutputMergerLogicOp);
				ADD_FEATURE(device_info, D3D11_OPTIONS, UAVOnlyRenderingForcedSampleCount);
				ADD_FEATURE(device_info, D3D11_OPTIONS, DiscardAPIsSeenByDriver);
				ADD_FEATURE(device_info, D3D11_OPTIONS, FlagsForUpdateAndCopySeenByDriver);
				ADD_FEATURE(device_info, D3D11_OPTIONS, ClearView);
				ADD_FEATURE(device_info, D3D11_OPTIONS, CopyWithOverlap);
				ADD_FEATURE(device_info, D3D11_OPTIONS, ConstantBufferPartialUpdate);
				ADD_FEATURE(device_info, D3D11_OPTIONS, ConstantBufferOffsetting);
				ADD_FEATURE(device_info, D3D11_OPTIONS, MapNoOverwriteOnDynamicConstantBuffer);
				ADD_FEATURE(device_info, D3D11_OPTIONS, MapNoOverwriteOnDynamicBufferSRV);
				ADD_FEATURE(device_info, D3D11_OPTIONS, MultisampleRTVWithForcedSampleCountOne);
				ADD_FEATURE(device_info, D3D11_OPTIONS, SAD4ShaderInstructions);
				ADD_FEATURE(device_info, D3D11_OPTIONS, ExtendedDoublesShaderInstructions);
				ADD_FEATURE(device_info, D3D11_OPTIONS, ExtendedResourceSharing);
			}

			if (DX11_QUERY_FEATURE(dx11_device, ARCHITECTURE_INFO))
			{
				ADD_FEATURE(device_info, ARCHITECTURE_INFO, TileBasedDeferredRenderer);
			}

			if (DX11_QUERY_FEATURE(dx11_device, D3D9_OPTIONS))
			{
				ADD_FEATURE(device_info, D3D9_OPTIONS, FullNonPow2TextureSupport);
			}

			if (DX11_QUERY_FEATURE(dx11_device, SHADER_MIN_PRECISION_SUPPORT))
			{
				ADD_FEATURE(device_info, SHADER_MIN_PRECISION_SUPPORT, PixelShaderMinPrecision);
				ADD_FEATURE(device_info, SHADER_MIN_PRECISION_SUPPORT, AllOtherShaderStagesMinPrecision);
			}

			if (DX11_QUERY_FEATURE(dx11_device, D3D9_SHADOW_SUPPORT))
			{
				ADD_FEATURE(device_info, D3D9_SHADOW_SUPPORT, SupportsDepthAsTextureWithLessEqualComparisonFilter);
			}

			if (DX11_QUERY_FEATURE(dx11_device, D3D11_OPTIONS1))
			{
				ADD_FEATURE(device_info, D3D11_OPTIONS1, TiledResourcesTier);
				ADD_FEATURE(device_info, D3D11_OPTIONS1, MinMaxFiltering);
				ADD_FEATURE(device_info, D3D11_OPTIONS1, ClearViewAlsoSupportsDepthOnlyFormats);
				ADD_FEATURE(device_info, D3D11_OPTIONS1, MapOnDefaultBuffers);
			}

			if (DX11_QUERY_FEATURE(dx11_device, D3D9_SIMPLE_INSTANCING_SUPPORT))
			{
				ADD_FEATURE(device_info, D3D9_SIMPLE_INSTANCING_SUPPORT, SimpleInstancingSupported);
			}

			if (DX11_QUERY_FEATURE(dx11_device, MARKER_SUPPORT))
			{
				AddFeature(device_info, "GPU Profiling Marker Support", MARKER_SUPPORT.Profile);
			}

			if (DX11_QUERY_FEATURE(dx11_device, D3D9_OPTIONS1))
			{
				ADD_FEATURE(device_info, D3D9_OPTIONS1, FullNonPow2TextureSupported);
				ADD_FEATURE(device_info, D3D9_OPTIONS1, DepthAsTextureWithLessEqualComparisonFilterSupported);
				ADD_FEATURE(device_info, D3D9_OPTIONS1, SimpleInstancingSupported);
				ADD_FEATURE(device_info, D3D9_OPTIONS1, TextureCubeFaceRenderTargetWithNonCubeDepthStencilSupported);
			}
#endif
#ifdef SYSTEMINFO_WIN10_AVAILABLE
			if (DX11_QUERY_FEATURE(dx11_device, D3D11_OPTIONS2))
			{
				ADD_FEATURE(device_info, D3D11_OPTIONS2, PSSpecifiedStencilRefSupported);
				ADD_FEATURE(device_info, D3D11_OPTIONS2, TypedUAVLoadAdditionalFormats);
				ADD_FEATURE(device_info, D3D11_OPTIONS2, ROVsSupported);
				ADD_FEATURE(device_info, D3D11_OPTIONS2, ConservativeRasterizationTier);
				//ADD_FEATURE(device_info, D3D11_OPTIONS2, TiledResourcesTier); // already queried from D3D11_FEATURE_D3D11_OPTIONS1
				ADD_FEATURE(device_info, D3D11_OPTIONS2, MapOnDefaultTextures);
				ADD_FEATURE(device_info, D3D11_OPTIONS2, StandardSwizzle);
				ADD_FEATURE(device_info, D3D11_OPTIONS2, UnifiedMemoryArchitecture);
			}

			if (DX11_QUERY_FEATURE(dx11_device, D3D11_OPTIONS3))
			{
				ADD_FEATURE(device_info, D3D11_OPTIONS3, VPAndRTArrayIndexFromAnyShaderFeedingRasterizer);
			}

			if (DX11_QUERY_FEATURE(dx11_device, GPU_VIRTUAL_ADDRESS_SUPPORT))
			{
				ADD_FEATURE(device_info, GPU_VIRTUAL_ADDRESS_SUPPORT, MaxGPUVirtualAddressBitsPerResource);
				ADD_FEATURE(device_info, GPU_VIRTUAL_ADDRESS_SUPPORT, MaxGPUVirtualAddressBitsPerProcess);
			}

			if (DX11_QUERY_FEATURE(dx11_device, D3D11_OPTIONS4))
			{
				ADD_FEATURE(device_info, D3D11_OPTIONS4, ExtendedNV12SharedTextureSupported);
			}
#endif

			// D3D11_FEATURE_DATA_FORMAT_SUPPORT and D3D11_FEATURE_DATA_FORMAT_SUPPORT2 is not
			// queried because it is a ~118x45 matrix and most of it is not useful for systeminfo
		}

		device_info.name = ws2s(AdapterDesc.Description);
		device_info.vendorID = AdapterDesc.VendorId;
		device_info.deviceID = AdapterDesc.DeviceId;
		ExtractFeatureLevelVersion(featureLevel, device_info.majorVersion, device_info.minorVersion);
		NGLOG_TRACE("DX11 device info name %s Version %s %s", device_info.name, device_info.majorVersion, device_info.minorVersion);

		assert(sizeof(device_info.luid) == sizeof(AdapterDesc.AdapterLuid));
		memcpy(&device_info.luid, &AdapterDesc.AdapterLuid, sizeof(device_info.luid));

		systemInfo.directxInfo.devices.push_back(device_info);
		systemInfo.hasDirectx = true;

		if (dx11_device != nullptr)
		{
			dx11_device->Release();
		}

		if (context != nullptr)
		{
			context->Release();
		}
	}
	else
	{
		NGLOG_TRACE("Unable to ran D3D11CreateDevice %s", hr);
	}
}
