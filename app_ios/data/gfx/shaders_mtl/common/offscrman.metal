#include <metal_graphics>
#include <metal_matrix>
#include <metal_geometric>
#include <metal_math>
#include <metal_texture>
#include <metal_common>

using namespace metal;


struct OffScreenInOut
{
    hfloat4 m_Position [[position]];
    v_float2 vTexCoord [[user(v_tex_coord)]];
};


struct VertexInput
{
    hfloat2 myVertex   [[attribute(0)]];
    hfloat2 myTexCoord [[attribute(1)]];
};


vertex OffScreenInOut vert1(VertexInput in [[ stage_in]])
{
    OffScreenInOut outVertices;
    
    outVertices.m_Position = hfloat4(in.myVertex.x, in.myVertex.y, 0.0, 1.0);
    outVertices.vTexCoord = v_float2(in.myTexCoord);
    
    return outVertices;
}


vertex OffScreenInOut vert2(VertexInput in [[ stage_in]])
{
    OffScreenInOut outVertices;
    
    outVertices.m_Position = hfloat4(in.myVertex.x, in.myVertex.y, 0.0, 1.0);
    outVertices.vTexCoord = v_float2(in.myTexCoord.y, 1.0-in.myTexCoord.x);
    
    return outVertices;
}


vertex OffScreenInOut virtres_vert1(VertexInput in [[ stage_in]])
{
    OffScreenInOut outVertices;
    
    outVertices.m_Position = hfloat4(in.myVertex.x, in.myVertex.y, 0.0, 1.0);
    outVertices.vTexCoord = v_float2(in.myTexCoord.x, 1.0-in.myTexCoord.y);
    
    return outVertices;
}


vertex OffScreenInOut virtres_vert2(VertexInput in [[ stage_in]])
{
    OffScreenInOut outVertices;
    
    outVertices.m_Position = hfloat4(in.myVertex.x, in.myVertex.y, 0.0, 1.0);
    outVertices.vTexCoord = v_float2(in.myTexCoord.y, in.myTexCoord.x);
    
    return outVertices;
}


constexpr sampler offscrsamp(coord::normalized, filter::linear, address::clamp_to_edge);



fragment _float4 frag(OffScreenInOut  inFrag    [[ stage_in ]],
					texture2d<_float> unit0 [[texture(0)]] )
{   
    return unit0.sample(offscrsamp, hfloat2(inFrag.vTexCoord));
}



