/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <d3d11_1.h>
#include <cstdio>
#include <string>

//////////////////////////////////////////////////////////////////////////////////////
// NOTE: The wrapper does not increase reference count for the object being wrapped //
// You need to keep a reference to the object yourself alongside with the warapper  //
//////////////////////////////////////////////////////////////////////////////////////

class D3D11DeviceContext1Wrapper: public ID3D11DeviceContext1
{
private:
	struct BlendStateStruct
	{
		ID3D11BlendState* pBlendState;
		FLOAT BlendFactor[ 4 ];
		UINT SampleMask;
	} currentBlendState, lastBlendState;

	struct DepthStencilStateStruct
	{
		ID3D11DepthStencilState *pDepthStencilState;
		UINT StencilRef;
	} currentDepthStencilState, lastDepthStencilState;

	ID3D11RasterizerState *currentRasterizerState, *lastRasterizerState;

	inline void ApplyStates()
	{
		if ( memcmp(&currentBlendState,&lastBlendState,sizeof(BlendStateStruct)))
		{
			innerobj->OMSetBlendState(currentBlendState.pBlendState,currentBlendState.BlendFactor,currentBlendState.SampleMask);
			memcpy(&lastBlendState, &currentBlendState, sizeof(BlendStateStruct));
		}
		if ( memcmp(&currentDepthStencilState, &lastDepthStencilState,sizeof(DepthStencilStateStruct)))
		{
			innerobj->OMSetDepthStencilState(currentDepthStencilState.pDepthStencilState,currentDepthStencilState.StencilRef);
			memcpy(&lastDepthStencilState, &currentDepthStencilState, sizeof(DepthStencilStateStruct));
		}
		if (currentRasterizerState != lastRasterizerState)
		{
			innerobj->RSSetState(currentRasterizerState);
			lastRasterizerState = currentRasterizerState;
		}
	}

	public:
		void EnableLogging()
		{
			loggingEnabled=true;
			drawcall=0;
			vertexcount=0;
			trianglecount=0;
		}

		void  DisableLogging()
		{
			loggingEnabled=false;
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476407(v=vs.85).aspx Draw
		virtual void STDMETHODCALLTYPE Draw( UINT VertexCount, UINT StartVertexLocation)
		{
			ApplyStates();
			LogDraw(VertexCount,1);
			innerobj->Draw(VertexCount,StartVertexLocation);
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476409(v=vs.85).aspx DrawIndexed
		virtual void STDMETHODCALLTYPE DrawIndexed( UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
		{
			ApplyStates();
			LogDraw(IndexCount,1);
			innerobj->DrawIndexed(IndexCount,StartIndexLocation, BaseVertexLocation);
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476410(v=vs.85).aspx DrawIndexedInstanced
		virtual void STDMETHODCALLTYPE DrawIndexedInstanced( UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
		{
			ApplyStates();
			LogDraw(IndexCountPerInstance,InstanceCount);
			innerobj->DrawIndexedInstanced(IndexCountPerInstance,InstanceCount,StartIndexLocation, BaseVertexLocation,StartInstanceLocation);
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476412(v=vs.85).aspx DrawInstanced
		virtual void STDMETHODCALLTYPE DrawInstanced( UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
		{
			ApplyStates();
			LogDraw(VertexCountPerInstance,InstanceCount);
			innerobj->DrawInstanced(VertexCountPerInstance,InstanceCount,StartVertexLocation,StartInstanceLocation);
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476408(v=vs.85).aspx DrawAuto
		virtual void STDMETHODCALLTYPE DrawAuto( void)
		{
			ApplyStates();
			// TODO: LogDraw?
			innerobj->DrawAuto();
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476413(v=vs.85).aspx DrawInstancedIndirect
		virtual void STDMETHODCALLTYPE DrawInstancedIndirect( ID3D11Buffer *pBufferForArgs, UINT AlignedByteOffsetForArgs)
		{
			ApplyStates();
			// pBufferForArgs is vertex data
			// TODO: LogDraw
			innerobj->DrawInstancedIndirect( pBufferForArgs,AlignedByteOffsetForArgs);
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476411(v=vs.85).aspx DrawIndexedInstancedIndirect
		virtual void STDMETHODCALLTYPE DrawIndexedInstancedIndirect( ID3D11Buffer *pBufferForArgs, UINT AlignedByteOffsetForArgs)
		{
			ApplyStates();
			// pBufferForArgs is index data
			// TODO: LogDraw
			innerobj->DrawIndexedInstancedIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476455(v=vs.85).aspx IASetPrimitiveTopology
		virtual void STDMETHODCALLTYPE IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY Topology)
		{
			topology = Topology;
			innerobj->IASetPrimitiveTopology( Topology);
		}

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476454(v=vs.85).aspx IASetInputLayout
		virtual void STDMETHODCALLTYPE IASetInputLayout( ID3D11InputLayout *pInputLayout)
		{
			innerobj->IASetInputLayout( pInputLayout);
		}

		virtual void STDMETHODCALLTYPE IAGetInputLayout( ID3D11InputLayout **ppInputLayout)
		{
			innerobj->IAGetInputLayout( ppInputLayout);
		}

		virtual void STDMETHODCALLTYPE OMSetBlendState( ID3D11BlendState *pBlendState, const FLOAT BlendFactor[ 4 ], UINT SampleMask)
		{
			const FLOAT DefaultBlendFactor[] = {1,1,1,1};
			if (BlendFactor==NULL)
			{
				BlendFactor=DefaultBlendFactor;
			}
			if (pBlendState != NULL) pBlendState->AddRef();
			if (currentBlendState.pBlendState != NULL) currentBlendState.pBlendState->Release();
			currentBlendState.pBlendState = pBlendState;
			memcpy(currentBlendState.BlendFactor, BlendFactor, sizeof(FLOAT) * 4);
			currentBlendState.SampleMask = SampleMask;
		}

		virtual void STDMETHODCALLTYPE OMGetBlendState( ID3D11BlendState **ppBlendState, FLOAT BlendFactor[ 4 ], UINT *pSampleMask)
		{
			if (currentBlendState.pBlendState != NULL) currentBlendState.pBlendState->AddRef();
			*ppBlendState = currentBlendState.pBlendState;
			memcpy(BlendFactor, currentBlendState.BlendFactor, sizeof(FLOAT) * 4);
			*pSampleMask = currentBlendState.SampleMask;
		}

		virtual void STDMETHODCALLTYPE OMSetDepthStencilState(ID3D11DepthStencilState *pDepthStencilState, UINT StencilRef)
		{
			if (pDepthStencilState != NULL) pDepthStencilState->AddRef();
			if (currentDepthStencilState.pDepthStencilState != NULL) currentDepthStencilState.pDepthStencilState->Release();
			currentDepthStencilState.pDepthStencilState = pDepthStencilState;
			currentDepthStencilState.StencilRef = StencilRef;
		}

		virtual void STDMETHODCALLTYPE OMGetDepthStencilState( ID3D11DepthStencilState **ppDepthStencilState, UINT *pStencilRef)
		{
			if (currentDepthStencilState.pDepthStencilState != NULL) currentDepthStencilState.pDepthStencilState->AddRef();
			*ppDepthStencilState = currentDepthStencilState.pDepthStencilState;
			*pStencilRef = currentDepthStencilState.StencilRef;
		}

		virtual void STDMETHODCALLTYPE RSSetState( ID3D11RasterizerState *pRasterizerState)
		{
			if (pRasterizerState != NULL) pRasterizerState->AddRef();
			if (currentRasterizerState != NULL) currentRasterizerState->Release();
			currentRasterizerState = pRasterizerState;
		}

		virtual void STDMETHODCALLTYPE RSGetState( ID3D11RasterizerState **ppRasterizerState)
		{
			if (currentRasterizerState != NULL) currentRasterizerState->AddRef();
			*ppRasterizerState = currentRasterizerState;
		}

		virtual void STDMETHODCALLTYPE ClearState( void)
		{
			//TODO: ez rendkvl fontos!
			innerobj->ClearState();
		}

		virtual void STDMETHODCALLTYPE PSSetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState *const *ppSamplers)
		{
			innerobj->PSSetSamplers(StartSlot,NumSamplers,ppSamplers);
		}

		virtual void STDMETHODCALLTYPE PSGetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState **ppSamplers)
		{
			innerobj->PSGetSamplers(StartSlot,NumSamplers, ppSamplers);
		}

		virtual void STDMETHODCALLTYPE VSSetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState *const *ppSamplers)
		{
			innerobj->VSSetSamplers(StartSlot,NumSamplers,ppSamplers);
		}

		virtual void STDMETHODCALLTYPE VSGetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState **ppSamplers)
		{
			innerobj->VSGetSamplers(StartSlot,NumSamplers, ppSamplers);
		}

		virtual void STDMETHODCALLTYPE GSSetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState *const *ppSamplers)
		{
			innerobj->GSSetSamplers(StartSlot,NumSamplers,ppSamplers);
		}

		virtual void STDMETHODCALLTYPE GSGetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState **ppSamplers)
		{
			innerobj->GSGetSamplers(StartSlot,NumSamplers, ppSamplers);
		}

		virtual void STDMETHODCALLTYPE HSSetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState *const *ppSamplers)
		{
			innerobj->HSSetSamplers(StartSlot,NumSamplers,ppSamplers);
		}

		virtual void STDMETHODCALLTYPE HSGetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState **ppSamplers)
		{
			innerobj->HSGetSamplers(StartSlot,NumSamplers, ppSamplers);
		}

		virtual void STDMETHODCALLTYPE DSSetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState *const *ppSamplers)
		{
			innerobj->DSSetSamplers(StartSlot,NumSamplers,ppSamplers);
		}

		virtual void STDMETHODCALLTYPE DSGetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState **ppSamplers)
		{
			innerobj->DSGetSamplers(StartSlot,NumSamplers, ppSamplers);
		}

		virtual void STDMETHODCALLTYPE CSSetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState *const *ppSamplers)
		{
			innerobj->CSSetSamplers(StartSlot,NumSamplers,ppSamplers);
		}

		virtual void STDMETHODCALLTYPE CSGetSamplers( UINT StartSlot, UINT NumSamplers, ID3D11SamplerState **ppSamplers)
		{
			innerobj->CSGetSamplers(StartSlot,NumSamplers,ppSamplers);
		}

		virtual void STDMETHODCALLTYPE CopySubresourceRegion1( ID3D11Resource *pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource *pSrcResource, UINT SrcSubresource, const D3D11_BOX *pSrcBox, UINT CopyFlags)
		{
			innerobj->CopySubresourceRegion1(pDstResource,DstSubresource,DstX,DstY,DstZ,pSrcResource,SrcSubresource,pSrcBox,CopyFlags);
		}

		virtual void STDMETHODCALLTYPE UpdateSubresource1( ID3D11Resource *pDstResource, UINT DstSubresource, const D3D11_BOX *pDstBox, const void *pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch, UINT CopyFlags)
		{
			innerobj->UpdateSubresource1(pDstResource,DstSubresource,pDstBox,pSrcData,SrcRowPitch,SrcDepthPitch,CopyFlags);
		}

		virtual void STDMETHODCALLTYPE DiscardResource( ID3D11Resource *pResource)
		{
			innerobj->DiscardResource(pResource);
		}

		virtual void STDMETHODCALLTYPE DiscardView(  ID3D11View *pResourceView)
		{
			innerobj->DiscardView(pResourceView);
		}

		virtual void STDMETHODCALLTYPE VSSetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers, const UINT *pFirstConstant, const UINT *pNumConstants)
		{
			innerobj-> VSSetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE HSSetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers, const UINT *pFirstConstant, const UINT *pNumConstants)
		{
			innerobj->HSSetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE DSSetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers, const UINT *pFirstConstant, const UINT *pNumConstants)
		{
			innerobj->DSSetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE GSSetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers, const UINT *pFirstConstant, const UINT *pNumConstants)
		{
			innerobj->GSSetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE PSSetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers, const UINT *pFirstConstant, const UINT *pNumConstants)
		{
			innerobj->PSSetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE CSSetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers, const UINT *pFirstConstant, const UINT *pNumConstants)
		{
			innerobj->CSSetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE VSGetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers, UINT *pFirstConstant, UINT *pNumConstants)
		{
			innerobj->VSGetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE HSGetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers, UINT *pFirstConstant, UINT *pNumConstants)
		{
			innerobj->HSGetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE DSGetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers, UINT *pFirstConstant, UINT *pNumConstants)
		{
			innerobj->DSGetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE GSGetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers, UINT *pFirstConstant, UINT *pNumConstants)
		{
			innerobj->GSGetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE PSGetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers, UINT *pFirstConstant, UINT *pNumConstants)
		{
			innerobj->PSGetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE CSGetConstantBuffers1( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers, UINT *pFirstConstant, UINT *pNumConstants)
		{
			innerobj->CSGetConstantBuffers1(StartSlot,NumBuffers,ppConstantBuffers,pFirstConstant,pNumConstants);
		}

		virtual void STDMETHODCALLTYPE SwapDeviceContextState( ID3DDeviceContextState *pState, ID3DDeviceContextState **ppPreviousState)
		{
			innerobj->SwapDeviceContextState( pState, ppPreviousState);
		}

		virtual void STDMETHODCALLTYPE ClearView( ID3D11View *pView, const FLOAT Color[ 4 ], const D3D11_RECT *pRect, UINT NumRects)
		{
			innerobj->ClearView(pView, Color, pRect, NumRects);
		}

		virtual void STDMETHODCALLTYPE DiscardView1( ID3D11View *pResourceView, const D3D11_RECT *pRects, UINT NumRects)
		{
			innerobj->DiscardView1( pResourceView, pRects,NumRects);
		}

		virtual void STDMETHODCALLTYPE VSSetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers)
		{
			innerobj->VSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE PSSetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews)
		{
			innerobj->PSSetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE PSSetShader( ID3D11PixelShader *pPixelShader, ID3D11ClassInstance *const *ppClassInstances, UINT NumClassInstances)
		{
			innerobj->PSSetShader( pPixelShader, ppClassInstances,NumClassInstances);
		}

		virtual void STDMETHODCALLTYPE VSSetShader( ID3D11VertexShader *pVertexShader, ID3D11ClassInstance *const *ppClassInstances, UINT NumClassInstances)
		{
			innerobj->VSSetShader( pVertexShader, ppClassInstances,NumClassInstances);
		}

		virtual HRESULT STDMETHODCALLTYPE Map( ID3D11Resource *pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE *pMappedResource)
		{
			return innerobj->Map( pResource,Subresource,MapType,MapFlags, pMappedResource);
		}

		virtual void STDMETHODCALLTYPE Unmap( ID3D11Resource *pResource, UINT Subresource)
		{
			innerobj->Unmap( pResource,Subresource);
		}

		virtual void STDMETHODCALLTYPE PSSetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers)
		{
			innerobj->PSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE IASetVertexBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppVertexBuffers, const UINT *pStrides, const UINT *pOffsets)
		{
			innerobj->IASetVertexBuffers(StartSlot,NumBuffers,ppVertexBuffers,pStrides,pOffsets);
		}

		virtual void STDMETHODCALLTYPE IASetIndexBuffer( ID3D11Buffer *pIndexBuffer, DXGI_FORMAT Format, UINT Offset)
		{
			innerobj->IASetIndexBuffer( pIndexBuffer, Format,Offset);
		}

		virtual void STDMETHODCALLTYPE GSSetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers)
		{
			innerobj->GSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE GSSetShader( ID3D11GeometryShader *pShader, ID3D11ClassInstance *const *ppClassInstances, UINT NumClassInstances)
		{
			innerobj->GSSetShader( pShader, ppClassInstances,NumClassInstances);
		}

		virtual void STDMETHODCALLTYPE VSSetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews)
		{
			innerobj->VSSetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE Begin( ID3D11Asynchronous *pAsync)
		{
			innerobj->Begin(pAsync);
		}

		virtual void STDMETHODCALLTYPE End( ID3D11Asynchronous *pAsync)
		{
			innerobj->End(pAsync);
		}

		virtual HRESULT STDMETHODCALLTYPE GetData( ID3D11Asynchronous *pAsync, void *pData, UINT DataSize, UINT GetDataFlags)
		{
			return innerobj->GetData( pAsync, pData,DataSize,GetDataFlags);
		}

		virtual void STDMETHODCALLTYPE SetPredication( ID3D11Predicate *pPredicate, BOOL PredicateValue)
		{
			innerobj->SetPredication( pPredicate, PredicateValue);
		}

		virtual void STDMETHODCALLTYPE GSSetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews)
		{
			innerobj->GSSetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE OMSetRenderTargets( UINT NumViews, ID3D11RenderTargetView *const *ppRenderTargetViews, ID3D11DepthStencilView *pDepthStencilView)
		{
			innerobj->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
		}

		virtual void STDMETHODCALLTYPE OMSetRenderTargetsAndUnorderedAccessViews( UINT NumRTVs, ID3D11RenderTargetView *const *ppRenderTargetViews, ID3D11DepthStencilView *pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView *const *ppUnorderedAccessViews, const UINT *pUAVInitialCounts)
		{
			innerobj->OMSetRenderTargetsAndUnorderedAccessViews(NumRTVs, ppRenderTargetViews, pDepthStencilView,UAVStartSlot,NumUAVs,ppUnorderedAccessViews,pUAVInitialCounts);
		}

		virtual void STDMETHODCALLTYPE SOSetTargets( UINT NumBuffers, ID3D11Buffer *const *ppSOTargets, const UINT *pOffsets)
		{
			innerobj->SOSetTargets(NumBuffers,ppSOTargets,pOffsets);
		}

		virtual void STDMETHODCALLTYPE Dispatch( UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
		{
			innerobj->Dispatch(ThreadGroupCountX,ThreadGroupCountY,ThreadGroupCountZ);
		}

		virtual void STDMETHODCALLTYPE DispatchIndirect( ID3D11Buffer *pBufferForArgs, UINT AlignedByteOffsetForArgs)
		{
			innerobj->DispatchIndirect( pBufferForArgs,AlignedByteOffsetForArgs);
		}

		virtual void STDMETHODCALLTYPE RSSetViewports( UINT NumViewports, const D3D11_VIEWPORT *pViewports)
		{
			innerobj->RSSetViewports(NumViewports, pViewports);
		}

		virtual void STDMETHODCALLTYPE RSSetScissorRects( UINT NumRects, const D3D11_RECT *pRects)
		{
			innerobj->RSSetScissorRects(NumRects, pRects);
		}

		virtual void STDMETHODCALLTYPE CopySubresourceRegion( ID3D11Resource *pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource *pSrcResource, UINT SrcSubresource, const D3D11_BOX *pSrcBox)
		{
			innerobj->CopySubresourceRegion( pDstResource,DstSubresource,DstX,DstY,DstZ, pSrcResource,SrcSubresource, pSrcBox);
		}

		virtual void STDMETHODCALLTYPE CopyResource( ID3D11Resource *pDstResource, ID3D11Resource *pSrcResource)
		{
			innerobj->CopyResource( pDstResource, pSrcResource);
		}

		virtual void STDMETHODCALLTYPE UpdateSubresource( ID3D11Resource *pDstResource, UINT DstSubresource, const D3D11_BOX *pDstBox, const void *pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch)
		{
			innerobj->UpdateSubresource( pDstResource,DstSubresource, pDstBox, pSrcData,SrcRowPitch,SrcDepthPitch);
		}

		virtual void STDMETHODCALLTYPE CopyStructureCount( ID3D11Buffer *pDstBuffer, UINT DstAlignedByteOffset, ID3D11UnorderedAccessView *pSrcView)
		{
			innerobj->CopyStructureCount( pDstBuffer,DstAlignedByteOffset, pSrcView);
		}

		virtual void STDMETHODCALLTYPE ClearRenderTargetView( ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[ 4 ])
		{
			innerobj->ClearRenderTargetView( pRenderTargetView, ColorRGBA);
		}

		virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewUint( ID3D11UnorderedAccessView *pUnorderedAccessView, const UINT Values[ 4 ])
		{
			innerobj->ClearUnorderedAccessViewUint( pUnorderedAccessView, Values);
		}

		virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewFloat( ID3D11UnorderedAccessView *pUnorderedAccessView, const FLOAT Values[ 4 ])
		{
			innerobj->ClearUnorderedAccessViewFloat( pUnorderedAccessView, Values );
		}

		virtual void STDMETHODCALLTYPE ClearDepthStencilView( ID3D11DepthStencilView *pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
		{
			innerobj->ClearDepthStencilView( pDepthStencilView,ClearFlags, Depth, Stencil);
		}

		virtual void STDMETHODCALLTYPE GenerateMips( ID3D11ShaderResourceView *pShaderResourceView)
		{
			innerobj->GenerateMips( pShaderResourceView);
		}

		virtual void STDMETHODCALLTYPE SetResourceMinLOD( ID3D11Resource *pResource, FLOAT MinLOD)
		{
			innerobj->SetResourceMinLOD( pResource, MinLOD);
		}

		virtual FLOAT STDMETHODCALLTYPE GetResourceMinLOD( ID3D11Resource *pResource)
		{
			return innerobj->GetResourceMinLOD( pResource);
		}

		virtual void STDMETHODCALLTYPE ResolveSubresource( ID3D11Resource *pDstResource, UINT DstSubresource, ID3D11Resource *pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format)
		{
			innerobj->ResolveSubresource( pDstResource,DstSubresource, pSrcResource,SrcSubresource, Format);
		}

		virtual void STDMETHODCALLTYPE ExecuteCommandList( ID3D11CommandList *pCommandList, BOOL RestoreContextState)
		{
			innerobj->ExecuteCommandList( pCommandList, RestoreContextState);
		}

		virtual void STDMETHODCALLTYPE HSSetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews)
		{
			innerobj->HSSetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE HSSetShader( ID3D11HullShader *pHullShader, ID3D11ClassInstance *const *ppClassInstances, UINT NumClassInstances)
		{
			innerobj->HSSetShader( pHullShader, ppClassInstances,NumClassInstances);
		}

		virtual void STDMETHODCALLTYPE HSSetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers)
		{
			innerobj->HSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE DSSetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews)
		{
			innerobj->DSSetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE DSSetShader( ID3D11DomainShader *pDomainShader, ID3D11ClassInstance *const *ppClassInstances, UINT NumClassInstances)
		{
			innerobj->DSSetShader( pDomainShader, ppClassInstances,NumClassInstances);
		}

		virtual void STDMETHODCALLTYPE DSSetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers)
		{
			innerobj->DSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE CSSetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews)
		{
			innerobj->CSSetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE CSSetUnorderedAccessViews( UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView *const *ppUnorderedAccessViews, const UINT *pUAVInitialCounts)
		{
			innerobj->CSSetUnorderedAccessViews(StartSlot,NumUAVs,ppUnorderedAccessViews,pUAVInitialCounts);
		}

		virtual void STDMETHODCALLTYPE CSSetShader( ID3D11ComputeShader *pComputeShader, ID3D11ClassInstance *const *ppClassInstances, UINT NumClassInstances)
		{
			innerobj->CSSetShader( pComputeShader, ppClassInstances,NumClassInstances);
		}

		virtual void STDMETHODCALLTYPE CSSetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer *const *ppConstantBuffers)
		{
			innerobj->CSSetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE VSGetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers)
		{
			innerobj->VSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE PSGetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView **ppShaderResourceViews)
		{
			innerobj->PSGetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE PSGetShader( ID3D11PixelShader **ppPixelShader, ID3D11ClassInstance **ppClassInstances, UINT *pNumClassInstances)
		{
			innerobj->PSGetShader(ppPixelShader, ppClassInstances,pNumClassInstances);
		}

		virtual void STDMETHODCALLTYPE VSGetShader( ID3D11VertexShader **ppVertexShader, ID3D11ClassInstance **ppClassInstances, UINT *pNumClassInstances)
		{
			innerobj->VSGetShader(ppVertexShader, ppClassInstances,pNumClassInstances);
		}

		virtual void STDMETHODCALLTYPE PSGetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers)
		{
			innerobj->PSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE IAGetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppVertexBuffers, UINT *pStrides, UINT *pOffsets)
		{
			innerobj->IAGetVertexBuffers(StartSlot,NumBuffers, ppVertexBuffers, pStrides, pOffsets);
		}

		virtual void STDMETHODCALLTYPE IAGetIndexBuffer( ID3D11Buffer **pIndexBuffer, DXGI_FORMAT *Format, UINT *Offset)
		{
			innerobj->IAGetIndexBuffer(pIndexBuffer, Format,Offset);
		}

		virtual void STDMETHODCALLTYPE GSGetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers)
		{
			innerobj->GSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE GSGetShader( ID3D11GeometryShader **ppGeometryShader, ID3D11ClassInstance **ppClassInstances, UINT *pNumClassInstances)
		{
			innerobj->GSGetShader( ppGeometryShader, ppClassInstances,pNumClassInstances);
		}

		virtual void STDMETHODCALLTYPE IAGetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY *pTopology)
		{
			innerobj->IAGetPrimitiveTopology( pTopology);
		}

		virtual void STDMETHODCALLTYPE VSGetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView **ppShaderResourceViews)
		{
			innerobj->VSGetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE GetPredication( ID3D11Predicate **ppPredicate, BOOL *pPredicateValue)
		{
			innerobj->GetPredication( ppPredicate, pPredicateValue);
		}

		virtual void STDMETHODCALLTYPE GSGetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView **ppShaderResourceViews)
		{
			innerobj->GSGetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE OMGetRenderTargets( UINT NumViews, ID3D11RenderTargetView **ppRenderTargetViews, ID3D11DepthStencilView **ppDepthStencilView)
		{
			innerobj->OMGetRenderTargets(NumViews, ppRenderTargetViews, ppDepthStencilView);
		}

		virtual void STDMETHODCALLTYPE OMGetRenderTargetsAndUnorderedAccessViews( UINT NumRTVs, ID3D11UnorderedAccessView **ppUnorderedAccessViews)
		{
			
		}
        virtual void STDMETHODCALLTYPE OMGetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView **ppRenderTargetViews, ID3D11DepthStencilView **ppDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView **ppUnorderedAccessViews)
		{
			innerobj->OMGetRenderTargetsAndUnorderedAccessViews(NumRTVs, ppRenderTargetViews,ppDepthStencilView,UAVStartSlot,NumUAVs,ppUnorderedAccessViews);
		}

		virtual void STDMETHODCALLTYPE SOGetTargets( UINT NumBuffers, ID3D11Buffer **ppSOTargets)
		{
			innerobj->SOGetTargets(NumBuffers,ppSOTargets);
		}

		virtual void STDMETHODCALLTYPE RSGetViewports(UINT *pNumViewports, D3D11_VIEWPORT *pViewports)
		{
			innerobj->RSGetViewports(pNumViewports,pViewports);
		}

		virtual void STDMETHODCALLTYPE RSGetScissorRects(UINT *pNumRects, D3D11_RECT *pRects)
		{
			innerobj->RSGetScissorRects(pNumRects, pRects);
		}

		virtual void STDMETHODCALLTYPE HSGetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView **ppShaderResourceViews)
		{
			innerobj->HSGetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE HSGetShader( ID3D11HullShader **ppHullShader, ID3D11ClassInstance **ppClassInstances, UINT *pNumClassInstances)
		{
			innerobj->HSGetShader( ppHullShader, ppClassInstances,pNumClassInstances);
		}

		virtual void STDMETHODCALLTYPE HSGetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers)
		{
			innerobj->HSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE DSGetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView **ppShaderResourceViews)
		{
			innerobj->DSGetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE DSGetShader( ID3D11DomainShader **ppDomainShader, ID3D11ClassInstance **ppClassInstances, UINT *pNumClassInstances)
		{
			innerobj->DSGetShader(ppDomainShader, ppClassInstances,pNumClassInstances);
		}

		virtual void STDMETHODCALLTYPE DSGetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers)
		{
			innerobj->DSGetConstantBuffers(StartSlot,NumBuffers,ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE CSGetShaderResources( UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView **ppShaderResourceViews)
		{
			innerobj->CSGetShaderResources(StartSlot,NumViews, ppShaderResourceViews);
		}

		virtual void STDMETHODCALLTYPE CSGetUnorderedAccessViews( UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView **ppUnorderedAccessViews)
		{
			innerobj->CSGetUnorderedAccessViews(StartSlot,NumUAVs, ppUnorderedAccessViews);
		}

		virtual void STDMETHODCALLTYPE CSGetShader( ID3D11ComputeShader **ppComputeShader, ID3D11ClassInstance **ppClassInstances, UINT *pNumClassInstances)
		{
			innerobj->CSGetShader(ppComputeShader,ppClassInstances,pNumClassInstances);
		}

		virtual void STDMETHODCALLTYPE CSGetConstantBuffers( UINT StartSlot, UINT NumBuffers, ID3D11Buffer **ppConstantBuffers)
		{
			innerobj->CSGetConstantBuffers( StartSlot, NumBuffers, ppConstantBuffers);
		}

		virtual void STDMETHODCALLTYPE Flush( void)
		{
			innerobj->Flush();
		}

		virtual D3D11_DEVICE_CONTEXT_TYPE STDMETHODCALLTYPE GetType( void)
		{
			return innerobj->GetType();
		}

		virtual UINT STDMETHODCALLTYPE GetContextFlags( void)
		{
			return innerobj->GetContextFlags();
		}

		virtual HRESULT STDMETHODCALLTYPE FinishCommandList( BOOL RestoreDeferredContextState, ID3D11CommandList **ppCommandList)
		{
			return innerobj->FinishCommandList(RestoreDeferredContextState,ppCommandList);
		}

		virtual void STDMETHODCALLTYPE GetDevice( ID3D11Device **ppDevice)
		{
			innerobj->GetDevice(ppDevice);
		}

		virtual HRESULT STDMETHODCALLTYPE GetPrivateData( REFGUID guid, UINT *pDataSize, void *pData)
		{
			return innerobj->GetPrivateData(guid,pDataSize,pData);
		}

		virtual HRESULT STDMETHODCALLTYPE SetPrivateData( REFGUID guid, UINT DataSize, const void *pData)
		{
			return innerobj->SetPrivateData(guid,DataSize,pData);
		}

		virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface( REFGUID guid, const IUnknown *pData)
		{
			return innerobj->SetPrivateDataInterface(guid,pData);
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
			}/* else if ( riid == IID_ID3D11DeviceContext1)
			{
				void* obj;
				HRESULT retval = ID3D11DeviceContext1::QueryInterface(riid,&obj);
				if (retval==NOERROR)
				{
					innerobj = (ID3D11DeviceContext1*)obj;
					AddRef();
					*ppvObject=this;
				}
				return retval;
			}*/ else
			{
				return E_NOINTERFACE;
			}
		}

		static D3D11DeviceContext1Wrapper* Wrap(ID3D11DeviceContext1* obj)
		{
			return new D3D11DeviceContext1Wrapper(obj);
		}

		ID3D11DeviceContext1* Unwrap()
		{
			ID3D11DeviceContext1 *retval=innerobj;
			if (currentRasterizerState != NULL)
			{
				currentRasterizerState->Release();
			}
			if (lastRasterizerState != NULL && lastRasterizerState != currentRasterizerState)
			{
				lastRasterizerState->Release();
			}
			if (currentBlendState.pBlendState != NULL)
			{
				currentBlendState.pBlendState->Release();
			}
			if (lastBlendState.pBlendState != NULL && lastBlendState.pBlendState != currentBlendState.pBlendState)
			{
				lastBlendState.pBlendState->Release();
			}
			if (currentDepthStencilState.pDepthStencilState != NULL)
			{
				currentDepthStencilState.pDepthStencilState->Release();
			}
			if (lastDepthStencilState.pDepthStencilState != NULL && lastDepthStencilState.pDepthStencilState != currentDepthStencilState.pDepthStencilState)
			{
				lastDepthStencilState.pDepthStencilState->Release();
			}
			currentBlendState.pBlendState = NULL;
			currentDepthStencilState.pDepthStencilState = NULL;
			currentRasterizerState = NULL;
			lastBlendState.pBlendState = NULL;
			lastDepthStencilState.pDepthStencilState = NULL;
			lastRasterizerState = NULL;
			innerobj = NULL;
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

		~D3D11DeviceContext1Wrapper()
		{
			if (innerobj != NULL)
			{
				Unwrap();
			}
		}

private:
	D3D11DeviceContext1Wrapper(ID3D11DeviceContext1 *obj)
	{
		innerobj = obj;
		loggingEnabled = false;
		topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		innerobj->IAGetPrimitiveTopology(&topology);
		innerobj->OMGetBlendState(&currentBlendState.pBlendState,currentBlendState.BlendFactor,&currentBlendState.SampleMask);
		innerobj->OMGetDepthStencilState(&currentDepthStencilState.pDepthStencilState,&currentDepthStencilState.StencilRef);
		innerobj->RSGetState(&currentRasterizerState);
		memcpy(&lastBlendState,&currentBlendState,sizeof(BlendStateStruct));
		memcpy(&lastDepthStencilState,&currentDepthStencilState,sizeof(DepthStencilStateStruct));
		lastRasterizerState = currentRasterizerState;
	}

	ID3D11DeviceContext1 *innerobj;

	UINT drawcall;
	UINT vertexcount;
	UINT trianglecount;
	D3D_PRIMITIVE_TOPOLOGY topology;
	BOOL loggingEnabled;

	void LogDraw(UINT VertexPerInstance, UINT InstanceCount)
	{
		if (loggingEnabled)
		{
			++drawcall;
			vertexcount += VertexPerInstance*InstanceCount;
			switch (topology)
			{
				case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
					trianglecount += (VertexPerInstance/3)*InstanceCount;
					break;
				case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
					trianglecount += (VertexPerInstance-2)*InstanceCount;
					break;
				default:
					break;
			}
		}
	}
};