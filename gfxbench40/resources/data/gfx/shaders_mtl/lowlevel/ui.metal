#include <metal_stdlib>
using namespace metal;

struct VertexUniforms
{
	hfloat4 u_scale;
	hfloat4 u_offset;
};


struct VertexInput
{
	hfloat3 a_vertex [[attribute(0)]];
	hfloat2 a_texCoord [[attribute(1)]];
};


struct UIInOut
{
    hfloat4 v_Position [[position]];
    v_float2 v_TexCoord ;
};


#ifdef TYPE_VERTEX
vertex UIInOut UIVertex(         VertexInput     in [[ stage_in ]],
                        constant VertexUniforms *vu [[ buffer(1) ]] )
{
	UIInOut res ;
	
    res.v_TexCoord = _float2(in.a_texCoord);
	_float2 scrCoords = _float2(in.a_vertex.xy) * _float2(vu->u_scale.xy) + _float2(vu->u_offset.xy);
	res.v_Position = hfloat4( hfloat2(scrCoords), in.a_vertex.z, 1.0);
    
    return res ;
}
#endif


#ifdef TYPE_FRAGMENT
fragment _float4 UIFragment(UIInOut          inFrag        [[ stage_in ]],
					       texture2d<_float> texture_unit0 [[texture(0)]],
					       sampler           sampler0      [[sampler(0)]] )
{
	_float4 res ;
	
	res = texture_unit0.sample(sampler0, hfloat2(inFrag.v_TexCoord.xy));

	return res ;
}
#endif

