#include <metal_stdlib>
using namespace metal;


struct VertexUniforms
{
    hfloat  u_rotation;
    hfloat2 u_position;
    hfloat2 u_scale;
    hfloat2 u_matrixSize;
    hfloat4 u_color0;
    hfloat4 u_color1;
    hfloat4 u_color2;
    hfloat4 u_color3;
    hfloat2 u_screenResolution;
};
 
 
struct CPUOverheadInOut
{
    hfloat4 m_Position [[position]];
    v_float4 v_Color [[user(vcolor)]];
};

struct VertexInput
{
    hfloat3 pos [[attribute(0)]];
    hfloat2 texCoord [[attribute(1)]];
};


#ifdef TYPE_VERTEX
_float2x2 rotateZ(_float alpha)
{
    return _float2x2(
                    _float2(cos(alpha), -sin(alpha)),
                    _float2(sin(alpha), cos(alpha)));
}

vertex CPUOverheadInOut cpuOverheadVertex(VertexInput in [[ stage_in]],
                                          constant VertexUniforms  *pVertexUniforms [[ buffer(1) ]] )
{
    CPUOverheadInOut outVertices;
  
    _float4 color01 = mix(_float4(pVertexUniforms->u_color0), _float4(pVertexUniforms->u_color1), _float(in.texCoord.x) );
    _float4 color23 = mix(_float4(pVertexUniforms->u_color2), _float4(pVertexUniforms->u_color3), _float(in.texCoord.x) );
    
    outVertices.v_Color = mix(color01, color23, _float(in.texCoord.y) );
    _float2x2 rot = rotateZ(pVertexUniforms->u_rotation);
    _float2 localCoord = _float2(in.pos.xy) * _float2(pVertexUniforms->u_scale) * rot * _float(10.0) / _float2(pVertexUniforms->u_screenResolution);
    _float2 offset = _float(2.0) * (_float2(pVertexUniforms->u_position) / _float2(pVertexUniforms->u_matrixSize) ) - _float2(1.0, 1.0);
    _float2 scrCoord = localCoord + offset;
    outVertices.m_Position = _float4(scrCoord, in.pos.z, _float(1.0));
    
    return outVertices;
}
#endif



#ifdef TYPE_FRAGMENT
fragment _float4 cpuOverheadFragment(CPUOverheadInOut  inFrag    [[ stage_in ]] )
{
    _float4 res = _float4(1.0,0.0,0.0,1.0) ;
    
    res = _float4( inFrag.v_Color.xyz,1.0) ;
    
    return res ;
}
#endif
