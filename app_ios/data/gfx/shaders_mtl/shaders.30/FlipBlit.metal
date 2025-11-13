
#include <metal_stdlib>

using namespace metal;

struct VertexOutput
{
	float4 position [[position]];
	float4 uv;
};

vertex VertexOutput vertex_main(constant float4* posXY_uvZW [[buffer(0)]],
								uint vid [[vertex_id]])
{
	VertexOutput out;
	out.position = float4(posXY_uvZW[vid].xy, 0.0, 1.0);
	out.uv = float4(posXY_uvZW[vid].zw, 0.0, 0.0);
	
	return out;
}

fragment float4 fragment_main(	VertexOutput input [[stage_in]],
								texture2d<float> bufferTex [[texture(0)]],
								sampler sam [[sampler(0)]])
{
	//return bufferTex.sample(sam, input.uv.xy);
	return float4(1.0,1.0, 0.0, 1.0);
}
