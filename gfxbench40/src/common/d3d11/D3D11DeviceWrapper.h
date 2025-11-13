/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <d3d11_1.h>

#define USE_STATE_CACHE

#if defined USE_STATE_CACHE

template<class T>
inline DWORD32 GetKeyType(T const &src)
{
	const DWORD32 size = sizeof(T);
	const BYTE* ptr = reinterpret_cast<const BYTE*>(&src);
	const DWORD32 offset_basis = 2166136261;
	const DWORD32 FNV_prime = 16777619;

	DWORD32 hash = offset_basis;
	for (DWORD32 i=0;i<size;++i,++ptr)
	{
		hash ^= *ptr;
		hash *= FNV_prime;
	}

	return hash;
}

#if 0
template<class T>
struct mem_cmp {
	bool operator() (T const &a, T const &b)
	{
		return memcmp(&a,&b,sizeof(T))>0;
	}
};
#endif
#else
template<class T> inline DWORD32 GetKeyType(T const &src) { return 0; }
#endif

//////////////////////////////////////////////////////////////////////////////////////
// NOTE: The wrapper does not increase reference count for the object being wrapped //
// You need to keep a reference to the object yourself alongside with the wrapper   //
//////////////////////////////////////////////////////////////////////////////////////

class D3D11Device1Wrapper: public ID3D11Device1
{
private:
	std::map<DWORD32,ID3D11SamplerState*> samplerstates;
	std::map<DWORD32,ID3D11DepthStencilState*> depthstencilstates;
	std::map<DWORD32,ID3D11BlendState*> blendstates;
	std::map<DWORD32,ID3D11BlendState1*> blendstates1;
	std::map<DWORD32,ID3D11RasterizerState*> rasterizerstates;
	std::map<DWORD32,ID3D11RasterizerState1*> rasterizerstates1;

	template <class KeyType, class ValueType>
	inline void ClearMap(std::map<KeyType, ValueType*> &in_map)
	{
		HRESULT result = S_OK;

		for (std::map<KeyType, ValueType*>::iterator iter = in_map.begin(); iter != in_map.end(); iter++)
		{
			static_cast<IUnknown*>(iter->second)->Release();
		}
		in_map.clear();
	}

#if defined USE_STATE_CACHE
	template <class KeyType, class ValueType>
	inline HRESULT TryGetFromMap(const std::map<KeyType,ValueType*> &in_map, const KeyType &key, ValueType** value)
	{
		HRESULT result = S_OK;

		std::map<KeyType,ValueType*>::const_iterator iter = in_map.find(key);
		if (iter!=in_map.end())
		{
			*value = iter->second;
			static_cast<IUnknown*>(iter->second)->AddRef();
			return S_OK;
		}
		return !S_OK;
	}

	template <class ValueType>
	inline void UpdateMap(std::map<DWORD32,ValueType*> &in_map, const DWORD32 &key, ValueType** value)
	{
		in_map[key]=*value;
		static_cast<IUnknown*>(*value)->AddRef();
	}
#else
	template <class ValueType> inline HRESULT TryGetFromMap(const std::map<DWORD32,ValueType*> &in_map, const DWORD32 &key, ValueType** value) { return !S_OK; }
	template <class ValueType> inline void UpdateMap(std::map<DWORD32,ValueType*> &in_map, const DWORD32 &hash, ValueType** value) { }
#endif

	public:
		virtual void STDMETHODCALLTYPE GetImmediateContext1( ID3D11DeviceContext1 **ppImmediateContext)
		{
			innerobj->GetImmediateContext1(ppImmediateContext);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateDeferredContext1( UINT ContextFlags,ID3D11DeviceContext1 **ppDeferredContext)
		{
			return innerobj->CreateDeferredContext1(ContextFlags,ppDeferredContext);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateBlendState1( const D3D11_BLEND_DESC1 *pBlendStateDesc,ID3D11BlendState1 **ppBlendState)
		{
			DWORD32 key = GetKeyType(*pBlendStateDesc);
			if (TryGetFromMap(blendstates1,key,ppBlendState)==S_OK) return S_OK;
			HRESULT result = innerobj->CreateBlendState1(pBlendStateDesc,ppBlendState);
			if (result==S_OK) UpdateMap(blendstates1,key,ppBlendState);
			return result;
		}

		virtual HRESULT STDMETHODCALLTYPE CreateRasterizerState1( const D3D11_RASTERIZER_DESC1 *pRasterizerDesc,ID3D11RasterizerState1 **ppRasterizerState)
		{
			DWORD32 key = GetKeyType(*pRasterizerDesc);
			if (TryGetFromMap(rasterizerstates1,key,ppRasterizerState)==S_OK) return S_OK;
			HRESULT result = innerobj->CreateRasterizerState1(pRasterizerDesc,ppRasterizerState);
			if (result==S_OK) UpdateMap(rasterizerstates1,key,ppRasterizerState);
			return result;
		}

		virtual HRESULT STDMETHODCALLTYPE CreateBlendState( const D3D11_BLEND_DESC *pBlendStateDesc,ID3D11BlendState **ppBlendState)
		{
			DWORD32 key = GetKeyType(*pBlendStateDesc);
			if (TryGetFromMap(blendstates,key,ppBlendState)==S_OK) return S_OK;
			HRESULT result = innerobj->CreateBlendState(pBlendStateDesc,ppBlendState);
			if (result==S_OK) UpdateMap(blendstates,key,ppBlendState);
			return result;
		}

		virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilState( const D3D11_DEPTH_STENCIL_DESC *pDepthStencilDesc,ID3D11DepthStencilState **ppDepthStencilState)
		{
			DWORD32 key = GetKeyType(*pDepthStencilDesc);
			if (TryGetFromMap(depthstencilstates,key,ppDepthStencilState)==S_OK) return S_OK;
			HRESULT result = innerobj->CreateDepthStencilState(pDepthStencilDesc,ppDepthStencilState);
			if (result==S_OK) UpdateMap(depthstencilstates,key,ppDepthStencilState);
			return result;
		}

		virtual HRESULT STDMETHODCALLTYPE CreateRasterizerState( const D3D11_RASTERIZER_DESC *pRasterizerDesc,ID3D11RasterizerState **ppRasterizerState)
		{
			DWORD32 key = GetKeyType(*pRasterizerDesc);
			if (TryGetFromMap(rasterizerstates,key,ppRasterizerState)==S_OK) return S_OK;
			HRESULT result = innerobj->CreateRasterizerState(pRasterizerDesc,ppRasterizerState);
			if (result==S_OK) UpdateMap(rasterizerstates,key,ppRasterizerState);
			return result;
		}

		virtual HRESULT STDMETHODCALLTYPE CreateSamplerState( const D3D11_SAMPLER_DESC *pSamplerDesc,ID3D11SamplerState **ppSamplerState)
		{
			DWORD32 key = GetKeyType(*pSamplerDesc);
			if (TryGetFromMap(samplerstates,key,ppSamplerState)==S_OK) return S_OK;
			HRESULT result = innerobj->CreateSamplerState(pSamplerDesc,ppSamplerState);
			if (result==S_OK) UpdateMap(samplerstates,key,ppSamplerState);
			return result;
		}

		virtual HRESULT STDMETHODCALLTYPE CreateDeviceContextState( UINT Flags, const D3D_FEATURE_LEVEL *pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, REFIID EmulatedInterface,D3D_FEATURE_LEVEL *pChosenFeatureLevel,ID3DDeviceContextState **ppContextState)
		{
			return innerobj->CreateDeviceContextState(Flags, pFeatureLevels, FeatureLevels, SDKVersion, EmulatedInterface, pChosenFeatureLevel,ppContextState);
		}

		virtual HRESULT STDMETHODCALLTYPE OpenSharedResource1( HANDLE hResource,REFIID returnedInterface,void **ppResource)
		{
			return innerobj->OpenSharedResource1(hResource,returnedInterface,ppResource);
		}

		virtual HRESULT STDMETHODCALLTYPE OpenSharedResourceByName( LPCWSTR lpName,DWORD dwDesiredAccess,REFIID returnedInterface,void **ppResource)
		{
			return innerobj->OpenSharedResourceByName(lpName,dwDesiredAccess,returnedInterface,ppResource);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateBuffer( const D3D11_BUFFER_DESC *pDesc,const D3D11_SUBRESOURCE_DATA *pInitialData,ID3D11Buffer **ppBuffer)
		{
			return innerobj->CreateBuffer( pDesc,pInitialData,ppBuffer);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateTexture1D( const D3D11_TEXTURE1D_DESC *pDesc, const D3D11_SUBRESOURCE_DATA *pInitialData,ID3D11Texture1D **ppTexture1D)
		{
			return innerobj->CreateTexture1D(pDesc,pInitialData,ppTexture1D);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateTexture2D( const D3D11_TEXTURE2D_DESC *pDesc, const D3D11_SUBRESOURCE_DATA *pInitialData,ID3D11Texture2D **ppTexture2D)
		{
			return innerobj->CreateTexture2D(pDesc,pInitialData,ppTexture2D);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateTexture3D( const D3D11_TEXTURE3D_DESC *pDesc, const D3D11_SUBRESOURCE_DATA *pInitialData,ID3D11Texture3D **ppTexture3D)
		{
			return innerobj->CreateTexture3D(pDesc,pInitialData,ppTexture3D);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateShaderResourceView( ID3D11Resource *pResource,const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc,ID3D11ShaderResourceView **ppSRView)
		{
			return innerobj->CreateShaderResourceView(pResource,pDesc,ppSRView);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateUnorderedAccessView( ID3D11Resource *pResource,const D3D11_UNORDERED_ACCESS_VIEW_DESC *pDesc,ID3D11UnorderedAccessView **ppUAView)
		{
			return innerobj->CreateUnorderedAccessView(pResource,pDesc,ppUAView);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateRenderTargetView( ID3D11Resource *pResource,const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,ID3D11RenderTargetView **ppRTView)
		{
			return innerobj->CreateRenderTargetView(pResource,pDesc,ppRTView);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilView( ID3D11Resource *pResource,const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,ID3D11DepthStencilView **ppDepthStencilView)
		{
			return innerobj->CreateDepthStencilView(pResource,pDesc,ppDepthStencilView);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateInputLayout(  const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, UINT NumElements,const void *pShaderBytecodeWithInputSignature,SIZE_T BytecodeLength,ID3D11InputLayout **ppInputLayout)
		{
			return innerobj->CreateInputLayout(pInputElementDescs,NumElements,pShaderBytecodeWithInputSignature,BytecodeLength,ppInputLayout);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateVertexShader( const void *pShaderBytecode,SIZE_T BytecodeLength,ID3D11ClassLinkage *pClassLinkage,ID3D11VertexShader **ppVertexShader)
		{
			return innerobj->CreateVertexShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppVertexShader);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateGeometryShader( const void *pShaderBytecode,SIZE_T BytecodeLength,ID3D11ClassLinkage *pClassLinkage,ID3D11GeometryShader **ppGeometryShader)
		{
			return innerobj->CreateGeometryShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppGeometryShader);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateGeometryShaderWithStreamOutput( const void *pShaderBytecode,SIZE_T BytecodeLength, const D3D11_SO_DECLARATION_ENTRY *pSODeclaration, UINT NumEntries, const UINT *pBufferStrides, UINT NumStrides,UINT RasterizedStream,ID3D11ClassLinkage *pClassLinkage,ID3D11GeometryShader **ppGeometryShader)
		{
			return innerobj->CreateGeometryShaderWithStreamOutput(pShaderBytecode,BytecodeLength,pSODeclaration,NumEntries,pBufferStrides,NumStrides,RasterizedStream,pClassLinkage,ppGeometryShader);
		}

		virtual HRESULT STDMETHODCALLTYPE CreatePixelShader( const void *pShaderBytecode,SIZE_T BytecodeLength,ID3D11ClassLinkage *pClassLinkage,ID3D11PixelShader **ppPixelShader)
		{
			return innerobj->CreatePixelShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppPixelShader);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateHullShader( const void *pShaderBytecode,SIZE_T BytecodeLength,ID3D11ClassLinkage *pClassLinkage,ID3D11HullShader **ppHullShader)
		{
			return innerobj->CreateHullShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppHullShader);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateDomainShader( const void *pShaderBytecode,SIZE_T BytecodeLength,ID3D11ClassLinkage *pClassLinkage,ID3D11DomainShader **ppDomainShader)
		{
			return innerobj->CreateDomainShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppDomainShader);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateComputeShader( const void *pShaderBytecode,SIZE_T BytecodeLength,ID3D11ClassLinkage *pClassLinkage,ID3D11ComputeShader **ppComputeShader)
		{
			return innerobj->CreateComputeShader(pShaderBytecode,BytecodeLength,pClassLinkage,ppComputeShader);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateClassLinkage( ID3D11ClassLinkage **ppLinkage)
		{
			return innerobj->CreateClassLinkage(ppLinkage);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateQuery( const D3D11_QUERY_DESC *pQueryDesc,ID3D11Query **ppQuery)
		{
			return innerobj->CreateQuery(pQueryDesc,ppQuery);
		}

		virtual HRESULT STDMETHODCALLTYPE CreatePredicate( const D3D11_QUERY_DESC *pPredicateDesc,ID3D11Predicate **ppPredicate)
		{
			return innerobj->CreatePredicate(pPredicateDesc,ppPredicate);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateCounter( const D3D11_COUNTER_DESC *pCounterDesc,ID3D11Counter **ppCounter)
		{
			return innerobj->CreateCounter(pCounterDesc,ppCounter);
		}

		virtual HRESULT STDMETHODCALLTYPE CreateDeferredContext( UINT ContextFlags,ID3D11DeviceContext **ppDeferredContext)
		{
			return innerobj->CreateDeferredContext(ContextFlags,ppDeferredContext);
		}

		virtual HRESULT STDMETHODCALLTYPE OpenSharedResource( HANDLE hResource,REFIID ReturnedInterface,void **ppResource)
		{
			return innerobj->OpenSharedResource(hResource,ReturnedInterface,ppResource);
		}

		virtual HRESULT STDMETHODCALLTYPE CheckFormatSupport( DXGI_FORMAT Format,UINT *pFormatSupport)
		{
			return innerobj->CheckFormatSupport(Format,pFormatSupport);
		}

		virtual HRESULT STDMETHODCALLTYPE CheckMultisampleQualityLevels( DXGI_FORMAT Format,UINT SampleCount,UINT *pNumQualityLevels)
		{
			return innerobj->CheckMultisampleQualityLevels(Format,SampleCount,pNumQualityLevels);
		}

		virtual void STDMETHODCALLTYPE CheckCounterInfo( D3D11_COUNTER_INFO *pCounterInfo)
		{
			innerobj->CheckCounterInfo(pCounterInfo);
		}

		virtual HRESULT STDMETHODCALLTYPE CheckCounter( const D3D11_COUNTER_DESC *pDesc,D3D11_COUNTER_TYPE *pType,UINT *pActiveCounters, LPSTR szName, UINT *pNameLength, LPSTR szUnits, UINT *pUnitsLength, LPSTR szDescription, UINT *pDescriptionLength)
		{
			return innerobj->CheckCounter(pDesc,pType,pActiveCounters,szName,pNameLength,szUnits,pUnitsLength,szDescription,pDescriptionLength);
		}

		virtual HRESULT STDMETHODCALLTYPE CheckFeatureSupport( D3D11_FEATURE Feature, void *pFeatureSupportData, UINT FeatureSupportDataSize)
		{
			return innerobj->CheckFeatureSupport(Feature,pFeatureSupportData,FeatureSupportDataSize);
		}

		virtual HRESULT STDMETHODCALLTYPE GetPrivateData( REFGUID guid, UINT *pDataSize, void *pData)
		{
			return innerobj->GetPrivateData(guid,pDataSize,pData);
		}

		virtual HRESULT STDMETHODCALLTYPE SetPrivateData( REFGUID guid,UINT DataSize, const void *pData)
		{
			return innerobj->SetPrivateData(guid,DataSize,pData);
		}

		virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface( REFGUID guid,const IUnknown *pData)
		{
			return innerobj->SetPrivateDataInterface(guid,pData);
		}

		virtual D3D_FEATURE_LEVEL STDMETHODCALLTYPE GetFeatureLevel( void)
		{
			return innerobj->GetFeatureLevel();
		}

		virtual UINT STDMETHODCALLTYPE GetCreationFlags( void)
		{
			return innerobj->GetCreationFlags();
		}

		virtual HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason( void)
		{
			return innerobj->GetDeviceRemovedReason();
		}

		virtual void STDMETHODCALLTYPE GetImmediateContext( ID3D11DeviceContext **ppImmediateContext)
		{
			innerobj->GetImmediateContext(ppImmediateContext);
		}

		virtual HRESULT STDMETHODCALLTYPE SetExceptionMode( UINT RaiseFlags)
		{
			return innerobj->SetExceptionMode( RaiseFlags);
		}

		virtual UINT STDMETHODCALLTYPE GetExceptionMode( void)
		{
			return innerobj->GetExceptionMode();
		}

		virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject)
		{
			if (!ppvObject) return E_INVALIDARG;
			*ppvObject = NULL;
			if (riid == IID_IUnknown)
			{
				*ppvObject = (LPVOID)this;
				AddRef();
				return NOERROR;
			} else
			{
				return E_NOINTERFACE;
			}
		}

		static D3D11Device1Wrapper* Wrap(ID3D11Device1* obj)
		{
			D3D11Device1Wrapper* retval= new D3D11Device1Wrapper();
			retval->innerobj = obj;
			return retval;
		}

		ID3D11Device1* Unwrap()
		{
			ID3D11Device1 *retval=innerobj;
			innerobj = NULL;
			ClearMap(samplerstates);
			ClearMap(depthstencilstates);
			ClearMap(blendstates);
			ClearMap(blendstates1);
			ClearMap(rasterizerstates);
			ClearMap(rasterizerstates1);
			return retval;
		}

		virtual ULONG STDMETHODCALLTYPE AddRef( void)
		{
			return innerobj->AddRef();
		}

		virtual ULONG STDMETHODCALLTYPE Release( void)
		{
			ULONG internalRC=innerobj->Release();
			if (internalRC==0)
			{
				innerobj = NULL;
				delete this;
			}
			return internalRC;
		}

		~D3D11Device1Wrapper()
		{
			if (innerobj != NULL) Unwrap();
		}
private:
	D3D11Device1Wrapper() {}
	ID3D11Device1 *innerobj;
};