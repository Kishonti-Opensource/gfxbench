#include <metal_stdlib>
using namespace metal;


struct VertexUniforms
{
	float2 texScale ;
	
	float __pad1 ;
	float __pad2 ;
};


struct NNMInOut
{
	float4 out_position [[position]];
	float2 out_texcoord0 ;
};


vertex NNMInOut NNMVertex(constant float4         *pPosition   [[ buffer(0) ]],
		   			      constant VertexUniforms *vu		   [[ buffer(1) ]],
		   			      uint                     vid         [[ vertex_id ]])
{
    NNMInOut r ;
    
    r.out_position = float4(pPosition[vid].xy,1.0,1.0) ;
    r.out_texcoord0 =  float2(0.5,0.5) + (2.0*pPosition[vid].zw - float2(1.0)) * vu->texScale ;
    
    return r;
}



fragment float4 NNMFragment(NNMInOut         inFrag    [[ stage_in ]],
							texture2d<float> texture_unit0 [[texture(0)]],
							sampler sampler0 [[sampler(0)]] )
{
	float4 res ;
	
	res = texture_unit0.sample(sampler0, inFrag.out_texcoord0);
    
    return res ;
}
