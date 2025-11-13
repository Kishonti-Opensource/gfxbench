/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_pipeline.h"

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

using namespace MetalRender;


Pipeline* Pipeline::CreateComputePipeline(const char *filename, const std::set<std::string> &defines_set, bool force_highp, bool tg_size)
{
    std::string header = "#define TYPE_compute 1\n" ;
    
    if (force_highp)
    {
        header += "#define FORCE_HIGHP 1\n" ;
        header += "#define VARYING_HIGHP 1\n" ;
    }
    else
    {
        header += "#define FORCE_HIGHP 0\n" ;
        header += "#define VARYING_HIGHP 0\n" ;
    }
    
    header.append( CollectDefines(defines_set) );
    
    LoadShaderResult cs = LoadShader(filename, header) ;
    
    NSError* err = nil;

	MTLComputePipelineDescriptor* desc = [[MTLComputePipelineDescriptor alloc] init];
    desc.computeFunction = cs.shaderFunction;
	desc.threadGroupSizeIsMultipleOfThreadExecutionWidth = tg_size;

    auto device = (id<MTLDevice>)MetalRender::GetContext()->getDevice();
	auto pipe = [device newComputePipelineStateWithDescriptor:desc options:0 reflection:nil error:&err];
    releaseObj(desc);

    if(!pipe)
    {
        NSLog(@"Error creating pipeline %@", [err localizedDescription]);
    }
    assert(pipe);
    
    Pipeline* p = new Pipeline();
    p->m_compute_pipeline = pipe;
    
    s_computePipeLines.insert(p);
    
    return p ;
}


void Pipeline::SetAsCompute(id <MTLComputeCommandEncoder> computeEncoder)
{
    [computeEncoder setComputePipelineState:m_compute_pipeline] ;
}


unsigned int Pipeline::GetMaxThreadCount()
{
    if (m_compute_pipeline == nil)
    {
        printf("ERROR! Not compute pipeline!!\n") ;
        assert(0);
        return 0;
    }
    
    return m_compute_pipeline.maxTotalThreadsPerThreadgroup;
}


bool Pipeline::IsThreadCountOk(const char* pass_name, unsigned int thread_count)
{
    unsigned int max_threads_per_group = GetMaxThreadCount();
    
    if (max_threads_per_group < thread_count)
    {
        printf("ERROR: %s computepass: desired worgroup size (%d) exceeds the maximum (%d)\n",pass_name,thread_count,max_threads_per_group);
        return false ;
    }
    return true;
}

