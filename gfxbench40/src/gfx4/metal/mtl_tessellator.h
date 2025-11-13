/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESSELLATOR_H
#define TESSELLATOR_H

#include "kcl_base.h"
#include "mtl_pipeline.h"
#include "mtl_dynamic_data_buffer.h"

#include <memory>
#include <functional>
#include <vector>



namespace MetalRender
{
    using std::pair;
    
    struct TessFactor
    {
        __fp16 Inner[4];
        __fp16 Outer[4];
    };
    
    struct ControlData
    {
        unsigned patchCount;
        unsigned instanceCount;
    };
    
    class Tessellator;
    struct TessellationAlgorithm;
    struct TessellationContext
    {
        ControlData controlData;
        
        unsigned numDrawCommands;
        unsigned primitiveType;
        unsigned indexCount;
        unsigned tgSize;
        bool instanced;
        TessellationAlgorithm* tessAlgo;
        
        
        id<MTLBuffer> indexBuffer;
        size_t indexBufferOffset;
        
        //User
        id<MTLBuffer> tessFactorsBuffer;
        size_t tessFactorsOffset;
        size_t perPatchDataOffset;
        size_t patchIndexBufferOffset;
        size_t drawPatchIndirectBufferOffset;
        size_t atomicBufferOffset;
        
        //Persistent
        size_t controlDataOffset;
        size_t drawIndirectCommandsOffset;
    };
    
    struct TessellationAlgorithm
    {
        virtual void IssueDraw(id<MTLRenderCommandEncoder> encoder, TessellationContext& ctx, Tessellator& tess) = 0;
        virtual void IssueCompute(id<MTLComputeCommandEncoder> encoder, TessellationContext& ctx, Tessellator& tess) = 0;
        
        TessellationAlgorithm() {};
        virtual ~TessellationAlgorithm() {};
    };
    
    class Tessellator
    {
    public:
        static bool UseStageInControlPoints();
        static bool ForceMaxTessFactorOne();
        
        Tessellator();
        virtual ~Tessellator();
        
        static MTLVertexDescriptor* GetVertexDescriptor();
        
        void Init(id<MTLDevice> device, unsigned maxTessLevels, MetalRender::DynamicDataBufferPool* pool);
        
		void EncodeIndirectBufferCopy(id <MTLCommandBuffer> command_buffer);
		
        void ResetTessellator();
        void ResetContextId();
        
        void PrepareIndexedPrimitives(id<MTLComputeCommandEncoder> encoder, unsigned tgSize, unsigned indexCount, unsigned patchVertices, unsigned instanceCount, bool instanced);
        void PrepareIndexedPrimitives(id<MTLComputeCommandEncoder> encoder, unsigned tgSize, unsigned indexCount, unsigned patchVertices, unsigned instanceCount, const TessFactor& factor, bool instanced);
        
        void IssueDraw(id<MTLRenderCommandEncoder> encoder, id<MTLBuffer> indexBuffer, size_t indexBufferOffset);
        
        
        enum PrimitiveType
        {
            FRACTIONAL_ODD_QUAD,
            FRACTIONAL_ODD_TRI,
            PRIMITIVE_TYPE_COUNT,
        };
        
        void ExecuteUserControlProgram(id<MTLComputeCommandEncoder> compute, TessellationContext& ctx);
        void ExecuteEmitDrawIndirectProgram(id<MTLComputeCommandEncoder> compute, TessellationContext& ctx);
        void ExecuteDraw(id<MTLRenderCommandEncoder> encoder, TessellationContext& ctx);
        
        size_t DrawCommandsForPatchCount(TessellationContext& ctx, unsigned patchCount);
        void EnsureDrawIndirectCommandsSize(TessellationContext& ctx, unsigned patchCount);
        void EnsureTessFactorsSize(TessellationContext& ctx, unsigned patchCount);
        void EnsurePerPatchDataSize(TessellationContext &ctx, unsigned int patchCount);
        void EnsureBufferSize(TessellationContext& ctx, unsigned patchCount);
        void EnsureControlData(TessellationContext& ctx, unsigned patchCount);
        
        
        TessellationContext& currentContext()
        {
            if(m_currentTessellationContextIdx >= m_tessellationContexts.size())
            {
                m_tessellationContexts.emplace_back();
            }
            
            return m_tessellationContexts[m_currentTessellationContextIdx];
        }
        
        std::unique_ptr<TessellationAlgorithm> m_tessellationAlgo;
    
        
        size_t m_currentDrawCommandsOffset;
        
        //Temp
        size_t m_currentInstanceCountOffset;
        size_t m_currentOffsetsOffset;
        size_t m_currentCommandIndexOffset;
        size_t m_currentDrawPatchIndirectCommandOffset;
        
        size_t m_maxPatchCount;
        size_t m_maxInstanceCount;
        size_t m_numberIndirectCommands = 0;
        
        unsigned m_currentTessellationContextIdx = 0;
        std::vector<TessellationContext> m_tessellationContexts;
        bool m_depthPass;
        
        MetalRender::DynamicDataBuffer* m_private_buffer;
        MetalRender::DynamicDataBuffer* m_managed_buffer;
		
#if !TARGET_OS_EMBEDDED
		MetalRender::DynamicDataBuffer* m_shared_indirect_buffer;
		MetalRender::DynamicDataBuffer* m_private_indirect_buffer;
		
		const uint32_t INDIRECT_COMMAND_BUFFER_SIZE = 64*1024;
#endif
    };
}



#endif //TESSELLATOR_H
