#include <metal_stdlib>
using namespace metal;

struct VertexUniforms
{
	hfloat u_rotation;
	hfloat u_aspectRatio;
	
	hfloat __pad1 ;
    hfloat __pad2 ;
};


struct VertexInput
{
	hfloat3 a_Vertex [[attribute(0)]];
	hfloat2 a_TexCoord [[attribute(1)]];
};


struct CompressedFillInOut
{
    hfloat4 v_Position [[position]];
    
    v_float2 v_texCoord0;
    v_float2 v_texCoord1;
    v_float2 v_texCoord2;
    v_float2 v_texCoord3;
};


_float2x2 rotateZ(_float alpha)
{
	return _float2x2(
	_float2(cos(alpha), -sin(alpha)),
	_float2(sin(alpha), cos(alpha))
	);
}


#ifdef TYPE_VERTEX
vertex CompressedFillInOut CompressedFillVertex(         VertexInput     in [[ stage_in ]],
                                                constant VertexUniforms *vu [[ buffer(1) ]] )
{
	CompressedFillInOut res ;
	
	_float2x2 rot0 = rotateZ( _float(vu->u_rotation));
	res.v_texCoord0 = (_float2(in.a_TexCoord) - _float2(0.5, 0.5)) * _float2(vu->u_aspectRatio * _float(2.0), _float(2.0)) * rot0
					  + _float2(0.5, 0.5);
	_float2x2 rot1 = rotateZ(-_float(vu->u_rotation));
	res.v_texCoord1 = _float2(in.a_TexCoord) * _float2(vu->u_aspectRatio, 1.0) * rot1;
	_float2x2 rot2 = rotateZ(-_float(vu->u_rotation) * _float(0.5));
	res.v_texCoord2 = _float2(in.a_TexCoord) * _float2(vu->u_aspectRatio, 1.0) * rot2;
	_float2x2 rot3 = rotateZ(_float(vu->u_rotation) * _float(0.5));
	res.v_texCoord3 = _float2(in.a_TexCoord) * _float2(vu->u_aspectRatio, 1.0) * rot3;
	res.v_Position = hfloat4(in.a_Vertex, 1.0);
    
    return res ;
}
#endif


#ifdef TYPE_FRAGMENT
fragment _float4 CompressedFillFragment(CompressedFillInOut inFrag        [[ stage_in ]],
					       texture2d<_float>    texture_unit0 [[texture(0)]],
					       sampler             sampler0      [[sampler(0)]],
					       texture2d<_float>    texture_unit1 [[texture(1)]],
					       sampler             sampler1      [[sampler(1)]] )
{
	_float4 res ;
	
	_float4 color = texture_unit0.sample(sampler0, hfloat2(inFrag.v_texCoord0));
	_float4 light1= texture_unit1.sample(sampler1, hfloat2(inFrag.v_texCoord1));
	_float4 light2= texture_unit1.sample(sampler1, hfloat2(inFrag.v_texCoord2));
	_float4 light3= texture_unit1.sample(sampler1, hfloat2(inFrag.v_texCoord3));
	_float4 light = light1 * _float(0.5) + light2 * _float(0.3) + light3 * _float(0.2);
	res = _float4(color.xyz * light.xyz, 1.0);
	
	return res ;
}
#endif

