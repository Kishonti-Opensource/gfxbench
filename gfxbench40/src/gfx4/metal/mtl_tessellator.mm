/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_tessellator.h"
#include "metal/mtl_pipeline_builder.h"
#include "kcl_io.h"
#include "mtl_shader_constant_layouts_40.h"
#include <algorithm>
#include "mtl_mesh4.h"

using namespace MetalRender;


#define ENABLE_EXTERNAL_PARAMS 0


struct TessellationAlgorithmCarChase : public TessellationAlgorithm
{
    virtual void IssueCompute(id<MTLComputeCommandEncoder> encoder, TessellationContext& ctx, Tessellator& tess)
    {
        return;
    }
    
    
    virtual void IssueDraw(id<MTLRenderCommandEncoder> encoder, TessellationContext& ctx, Tessellator& tess)
    {

        [encoder setVertexBuffer:tess.m_private_buffer->GetCurrentBuffer() offset:ctx.perPatchDataOffset atIndex:USER_PER_PATCH_SLOT];
        auto tessellationFactorSize = sizeof(uint16_t) * (ctx.primitiveType == Tessellator::FRACTIONAL_ODD_TRI ? 4 : 6);
        if(tess.m_depthPass)
        {
            tessellationFactorSize = 0;
        }
        [encoder setTessellationFactorBuffer:ctx.tessFactorsBuffer offset:ctx.tessFactorsOffset instanceStride:ctx.controlData.patchCount * tessellationFactorSize];
        
#if TARGET_OS_EMBEDDED
        if(ctx.primitiveType == Tessellator::FRACTIONAL_ODD_TRI)
        {
            if(Tessellator::UseStageInControlPoints())
            {
                [encoder drawIndexedPatches:3
                                 patchStart:0 patchCount:ctx.controlData.patchCount
                           patchIndexBuffer:nil patchIndexBufferOffset:0
                    controlPointIndexBuffer:ctx.indexBuffer controlPointIndexBufferOffset:ctx.indexBufferOffset
                              instanceCount:ctx.controlData.instanceCount baseInstance:0];
            }
            else
            {
                [encoder drawPatches:0
                          patchStart:0 patchCount:ctx.controlData.patchCount
                    patchIndexBuffer:nil patchIndexBufferOffset:0
                       instanceCount:ctx.controlData.instanceCount baseInstance:0];
            }
        }
        else
        {
            [encoder drawPatches:0
                      patchStart:0 patchCount:ctx.controlData.patchCount
                patchIndexBuffer:nil patchIndexBufferOffset:0
                   instanceCount:ctx.controlData.instanceCount baseInstance:0];
        }
#else
        if(!ctx.instanced && !tess.m_depthPass && ctx.primitiveType == Tessellator::FRACTIONAL_ODD_TRI)
        {
            if(Tessellator::UseStageInControlPoints())
            {
                [encoder drawIndexedPatches:3
                           patchIndexBuffer:tess.m_private_buffer->GetCurrentBuffer() patchIndexBufferOffset:ctx.patchIndexBufferOffset
                    controlPointIndexBuffer:ctx.indexBuffer controlPointIndexBufferOffset:ctx.indexBufferOffset
                             indirectBuffer:tess.m_private_indirect_buffer->GetCurrentBuffer() indirectBufferOffset:ctx.drawPatchIndirectBufferOffset];
            }
            else
            {
                [encoder drawPatches:0
                    patchIndexBuffer:tess.m_private_buffer->GetCurrentBuffer() patchIndexBufferOffset:ctx.patchIndexBufferOffset
                      indirectBuffer:tess.m_private_indirect_buffer->GetCurrentBuffer() indirectBufferOffset:ctx.drawPatchIndirectBufferOffset];
            }
        }
        else if(ctx.primitiveType == Tessellator::FRACTIONAL_ODD_TRI)
        {
            if(Tessellator::UseStageInControlPoints())
            {
                [encoder drawIndexedPatches:3
                           patchIndexBuffer:nil patchIndexBufferOffset:0
                    controlPointIndexBuffer:ctx.indexBuffer controlPointIndexBufferOffset:ctx.indexBufferOffset
                             indirectBuffer:tess.m_private_indirect_buffer->GetCurrentBuffer() indirectBufferOffset:ctx.drawPatchIndirectBufferOffset];
            }
            else
            {
                [encoder drawPatches:0
                    patchIndexBuffer:nil patchIndexBufferOffset:0
                      indirectBuffer:tess.m_private_indirect_buffer->GetCurrentBuffer() indirectBufferOffset:ctx.drawPatchIndirectBufferOffset];
            }
        }
        else
        {
            [encoder drawPatches:0
                patchIndexBuffer:nil patchIndexBufferOffset:0
                  indirectBuffer:tess.m_private_indirect_buffer->GetCurrentBuffer() indirectBufferOffset:ctx.drawPatchIndirectBufferOffset];

        }
#endif
    }
};

bool Tessellator::UseStageInControlPoints()
{
    static bool stageIn = true;
    
#if ENABLE_EXTERNAL_PARAMS
	static bool initialized = false;
    if(!initialized)
    {
        const char * stageInEnv = getenv("STAGEIN_CONTROLPOINTS");
        
        if (stageInEnv)
            stageIn = stageInEnv[0] == '1';
        
        initialized = true;
    }
#endif
    return stageIn;
}

bool Tessellator::ForceMaxTessFactorOne()
{
    static bool maxTessOne = false;
    
#if ENABLE_EXTERNAL_PARAMS
	static bool initialized = false;
    if(!initialized)
    {
        const char * maxTessOneEnv = getenv("MAX_ONE_TESS");
        
        if (maxTessOneEnv)
            maxTessOne = maxTessOneEnv[0] == '1';
        
        initialized = true;
    }
#endif
    return maxTessOne;
}

MTLVertexDescriptor* Tessellator::GetVertexDescriptor()
{
    static MTLVertexDescriptor *descriptor = nil;
    if(UseStageInControlPoints())
    {
        if(descriptor == nil)
        {
            descriptor = [GFXB4::Mesh3::GetVertexDescriptor() copy];
            
            descriptor.attributes[0].bufferIndex = VERTEX_BUFFER_SLOT;
            descriptor.attributes[1].bufferIndex = VERTEX_BUFFER_SLOT;
            descriptor.attributes[2].bufferIndex = VERTEX_BUFFER_SLOT;
            descriptor.attributes[3].bufferIndex = VERTEX_BUFFER_SLOT;
            descriptor.attributes[4].bufferIndex = VERTEX_BUFFER_SLOT;
            
            descriptor.layouts[VERTEX_BUFFER_SLOT] = descriptor.layouts[0];
            descriptor.layouts[VERTEX_BUFFER_SLOT].stepFunction = MTLVertexStepFunctionPerPatchControlPoint;
            descriptor.layouts[0] = nil;
        }
    }
    return descriptor;
}

Tessellator::Tessellator()
{
    m_depthPass = false;
}

Tessellator::~Tessellator()
{
}

void Tessellator::Init(id<MTLDevice> device, unsigned maxTessLevels, MetalRender::DynamicDataBufferPool* pool)
{
#if !TARGET_OS_EMBEDDED
    m_managed_buffer = pool->GetNewBuffer(1024 * 1024, MTLResourceStorageModeManaged);
	
	m_shared_indirect_buffer = pool->GetNewBuffer(INDIRECT_COMMAND_BUFFER_SIZE, MTLResourceStorageModeShared);
	m_private_indirect_buffer = pool->GetNewBuffer(INDIRECT_COMMAND_BUFFER_SIZE, MTLResourceStorageModePrivate);;
#else
    m_managed_buffer = pool->GetNewBuffer(1024 * 1024, MTLResourceStorageModeShared);
#endif
    m_private_buffer = pool->GetNewBuffer(1024 * 1024 * 20, MTLResourceStorageModePrivate);
    
    m_tessellationAlgo.reset(new TessellationAlgorithmCarChase());
}

void Tessellator::ResetTessellator()
{
    m_maxPatchCount = 0;
    m_maxInstanceCount = 0;
    m_currentDrawCommandsOffset = 0;
    m_currentCommandIndexOffset = 0;
    
    m_tessellationContexts.clear();
    m_currentTessellationContextIdx = 0;
}

void Tessellator::ResetContextId()
{
    m_currentTessellationContextIdx = 0;
}


void Tessellator::IssueDraw(id<MTLRenderCommandEncoder> encoder, id<MTLBuffer> indexBuffer, size_t indexBufferOffset)
{
    auto& ctx = currentContext();
    
    ctx.indexBuffer = indexBuffer;
    ctx.indexBufferOffset = indexBufferOffset;
    assert(ctx.indexBufferOffset == 0);
    
    ctx.tessAlgo->IssueDraw(encoder, ctx, *this);
    m_currentTessellationContextIdx++;
}

void Tessellator::ExecuteDraw(id<MTLRenderCommandEncoder> encoder, MetalRender::TessellationContext &ctx)
{
    [encoder setVertexBuffer:m_managed_buffer->GetCurrentBuffer() offset:ctx.controlDataOffset atIndex:CONTROL_DATA_SLOT];
    [encoder setVertexBuffer:m_private_buffer->GetCurrentBuffer() offset:ctx.perPatchDataOffset atIndex:USER_PER_PATCH_SLOT];
    [encoder setVertexBuffer:m_private_buffer->GetCurrentBuffer() offset:m_currentInstanceCountOffset atIndex:DRAW_IDX_COUNT_SLOT];
    [encoder setVertexBuffer:m_private_buffer->GetCurrentBuffer() offset:m_currentOffsetsOffset atIndex:DRAW_IDX_OFFSET_SLOT];
}

void Tessellator::PrepareIndexedPrimitives(id<MTLComputeCommandEncoder> encoder, unsigned tgSize, unsigned int indexCount, unsigned int patchVertices, unsigned int instanceCount, bool instanced)
{
    auto& ctx = currentContext();
    ctx.instanced = instanced;
    
    const auto patchCount = indexCount / patchVertices;
    
    
    ctx.controlData.patchCount = patchCount;
    ctx.controlData.instanceCount = instanceCount;
    
    m_maxPatchCount = MAX(m_maxPatchCount, ctx.controlData.patchCount);
    m_maxInstanceCount = MAX(m_maxInstanceCount, ctx.controlData.instanceCount);
    
    ctx.tessAlgo = m_tessellationAlgo.get();
    ctx.numDrawCommands = 90;
    ctx.indexCount = indexCount;
    ctx.tgSize = tgSize;
    ctx.tessFactorsBuffer = m_private_buffer->GetCurrentBuffer();
     

    switch(patchVertices)
    {
        case 16:
            ctx.primitiveType = FRACTIONAL_ODD_QUAD;
            break;
        case 3:
            ctx.primitiveType = FRACTIONAL_ODD_TRI;
            break;
        default:
            assert(0);
    }
    
    assert(patchCount);
    
    
#if !TARGET_OS_EMBEDDED
    ctx.patchIndexBufferOffset = -1;
    MTLDrawPatchIndirectArguments zero = {ctx.controlData.patchCount, ctx.controlData.instanceCount, 0, 0};
    if(!ctx.instanced)
        zero.patchCount = 0;
    
    ctx.drawPatchIndirectBufferOffset = m_shared_indirect_buffer->WriteDataAndGetOffset(nil, &zero, sizeof(zero));
    if(ctx.primitiveType == Tessellator::FRACTIONAL_ODD_TRI && !ctx.instanced)
        ctx.patchIndexBufferOffset = m_private_buffer->ReserveAndGetOffset(sizeof(unsigned) * ctx.controlData.patchCount);
#endif    
    
    EnsureControlData(ctx, patchCount);
    EnsureTessFactorsSize(ctx, patchCount);
    EnsurePerPatchDataSize(ctx, patchCount);
    
    ExecuteUserControlProgram(encoder, ctx);
    
    m_currentTessellationContextIdx++;
}

void Tessellator::PrepareIndexedPrimitives(id<MTLComputeCommandEncoder> encoder, unsigned tgSize, unsigned int indexCount, unsigned int patchVertices, unsigned int instanceCount, const MetalRender::TessFactor &factor, bool instanced)
{
    auto& ctx = currentContext();
    ctx.instanced = instanced;
    const auto patchCount = indexCount / patchVertices;
    
    
    switch(patchVertices)
    {
        case 16:
            ctx.primitiveType = FRACTIONAL_ODD_QUAD;
            ctx.numDrawCommands = 5;
            break;
        case 3:
            ctx.primitiveType = FRACTIONAL_ODD_TRI;
            ctx.numDrawCommands = 4;
            break;
        default:
            assert(0);
    }
    
    
    ctx.controlData.patchCount = patchCount;
    ctx.controlData.instanceCount = instanceCount;
    
    m_maxPatchCount = MAX(m_maxPatchCount, ctx.controlData.patchCount);
    m_maxInstanceCount = MAX(m_maxInstanceCount, ctx.controlData.instanceCount);
    
    ctx.indexCount = indexCount;
    ctx.tessFactorsBuffer = m_managed_buffer->GetCurrentBuffer();
    ctx.tessFactorsOffset = m_managed_buffer->WriteDataAndGetOffset(nil, &factor, sizeof(factor));
    ctx.tessAlgo = m_tessellationAlgo.get();
    ctx.tgSize = tgSize;
    
    assert(patchCount);
    
#if !TARGET_OS_EMBEDDED
    MTLDrawPatchIndirectArguments zero = {ctx.controlData.patchCount, ctx.controlData.instanceCount, 0, 0};
    
    ctx.drawPatchIndirectBufferOffset = m_shared_indirect_buffer->WriteDataAndGetOffset(nil, &zero, sizeof(zero));
#endif
    
    EnsurePerPatchDataSize(ctx, patchCount);
    EnsureControlData(ctx, patchCount);
    
    ExecuteUserControlProgram(encoder, ctx);
    
    m_currentTessellationContextIdx++;
}


void Tessellator::EnsureBufferSize(TessellationContext &ctx, unsigned int patchCount)
{
    EnsureDrawIndirectCommandsSize(ctx, patchCount);
    EnsureTessFactorsSize(ctx, patchCount);
    EnsurePerPatchDataSize(ctx, patchCount);
    EnsureControlData(ctx, patchCount);
}

void Tessellator::ExecuteUserControlProgram(id<MTLComputeCommandEncoder> compute, TessellationContext& ctx)
{
    [compute setBuffer:ctx.tessFactorsBuffer offset:ctx.tessFactorsOffset atIndex:TESS_FACTORS_SLOT];
    [compute setBuffer:m_private_buffer->GetCurrentBuffer() offset:ctx.perPatchDataOffset atIndex:USER_PER_PATCH_SLOT];
    [compute setBuffer:m_managed_buffer->GetCurrentBuffer() offset:ctx.controlDataOffset atIndex:CONTROL_DATA_SLOT];
    
#if !TARGET_OS_EMBEDDED
        [compute setBuffer:m_private_indirect_buffer->GetCurrentBuffer() offset:ctx.drawPatchIndirectBufferOffset atIndex:DRAW_PATCH_INDIRECT_SLOT];
        if(ctx.primitiveType == Tessellator::FRACTIONAL_ODD_TRI && !ctx.instanced)
        {
            [compute setBuffer:m_private_buffer->GetCurrentBuffer() offset:ctx.patchIndexBufferOffset atIndex:COMMAND_IDX_SLOT];
        }
#endif
    
    if(ctx.primitiveType == FRACTIONAL_ODD_TRI && m_depthPass)
        return;

    auto workgroupSize = MTLSizeMake(ctx.tgSize, 1, 1);
    auto workgroupSizeWidth1 = workgroupSize.width - 1;
    auto multiplier = ctx.primitiveType == FRACTIONAL_ODD_QUAD ? 16 : 1;
    
    
    auto gridSize = MTLSizeMake(((ctx.controlData.patchCount * ctx.controlData.instanceCount * multiplier + workgroupSizeWidth1) & ~workgroupSizeWidth1) / workgroupSize.width, 1, 1);
//    NSLog(@"patchCount : %d\n instanceCount : %d\nmulti : %d\ngridX : %d\n", ctx.controlData.patchCount, ctx.controlData.instanceCount, multiplier, (int)gridSize.width);
//    assert(multiplier != 16 || !ctx.instanced);

    [compute dispatchThreadgroups:gridSize threadsPerThreadgroup:workgroupSize];
 }

void Tessellator::ExecuteEmitDrawIndirectProgram(id<MTLComputeCommandEncoder> compute, TessellationContext& ctx)
{
    [compute setBuffer:m_managed_buffer->GetCurrentBuffer() offset:ctx.controlDataOffset atIndex:CONTROL_DATA_SLOT];
    [compute setBuffer:m_managed_buffer->GetCurrentBuffer() offset:ctx.drawIndirectCommandsOffset atIndex:DRAWS_SLOT];
    [compute setBuffer:m_private_buffer->GetCurrentBuffer()  offset:m_currentInstanceCountOffset atIndex:DRAW_IDX_COUNT_SLOT];
    [compute setBuffer:m_private_buffer->GetCurrentBuffer()  offset:m_currentOffsetsOffset atIndex:DRAW_IDX_OFFSET_SLOT];
}

size_t Tessellator::DrawCommandsForPatchCount(TessellationContext& ctx, unsigned patchCount)
{
    switch(ctx.primitiveType)
    {
        case FRACTIONAL_ODD_QUAD:
            return ctx.numDrawCommands;
        case FRACTIONAL_ODD_TRI:
            return ctx.numDrawCommands;
        default:
            assert(0);
    }
}

void Tessellator::EnsureDrawIndirectCommandsSize(TessellationContext& ctx, unsigned patchCount)
{
    auto newBufferSize = DrawCommandsForPatchCount(ctx, patchCount) * sizeof(MTLDrawIndexedPrimitivesIndirectArguments);
    
    ctx.drawIndirectCommandsOffset = m_currentDrawCommandsOffset;
    m_currentDrawCommandsOffset += newBufferSize;
}

void Tessellator::EnsureTessFactorsSize(TessellationContext& ctx, unsigned patchCount)
{
    auto newBufferSize = sizeof(TessFactor) * ctx.controlData.patchCount * ctx.controlData.instanceCount;
    
    ctx.tessFactorsOffset = m_private_buffer->ReserveAndGetOffset(newBufferSize);
}

void Tessellator::EnsurePerPatchDataSize(TessellationContext &ctx, unsigned int patchCount)
{
    if(ctx.primitiveType == FRACTIONAL_ODD_TRI)
        return;
    
    auto newBufferSize = sizeof(KCL::Matrix4x4) * ctx.controlData.patchCount * ctx.controlData.instanceCount * 3;
    ctx.perPatchDataOffset = m_private_buffer->ReserveAndGetOffset(newBufferSize);
}

void Tessellator::EnsureControlData(TessellationContext& ctx, unsigned patchCount)
{
    ctx.controlDataOffset = m_managed_buffer->WriteDataAndGetOffset(nil, &ctx.controlData, sizeof(ctx.controlData));
}


void Tessellator::EncodeIndirectBufferCopy(id <MTLCommandBuffer> command_buffer)
{
#if !TARGET_OS_EMBEDDED
	id <MTLBlitCommandEncoder> blitEncoder = [command_buffer blitCommandEncoder];
	[blitEncoder copyFromBuffer:m_shared_indirect_buffer->GetCurrentBuffer()
				   sourceOffset:0
					   toBuffer:m_private_indirect_buffer->GetCurrentBuffer()
			  destinationOffset:0
						   size:INDIRECT_COMMAND_BUFFER_SIZE];
	[blitEncoder endEncoding];
#endif
}

